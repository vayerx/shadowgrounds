
#include "precompiled.h"

#include "Ani.h"
//#include "AniManager.h" // necessary?

#include "Unit.h"
#include "UnitType.h"
#include "UnitActor.h"
#include "unittypes.h"
#include "scaledefs.h"
#include "Weapon.h"
#include "../ui/AnimationSet.h"

#include "ParticleSpawner.h"
#include "ParticleSpawnerManager.h"

#include "../convert/str2int.h"
#include "../system/Logger.h"
#include "../util/ScriptProcess.h"
#include "../util/Script.h"
#include "../util/SimpleParser.h"
#include "scripting/GameScripting.h"
#include "../ui/Animator.h"

#include <time.h>

#include "../util/Debug_MemoryManager.h"

// initial alloc is 64k
#define ANI_INITIAL_RECORD_BUF_ALLOC 65536

// about 6 meg max record size
#define ANI_MAX_RECORD_BUF_ALLOC 65536*100

#define ANI_MAX_MARK_LEN 64




namespace game
{
	class AniImpl
	{
		private:
			
			AniImpl(GameScripting *gameScripting, Unit *unit)
			{
				this->gameScripting = gameScripting;
				this->unit = unit;
				this->name = NULL;
				this->recording = false;
				this->playing = false;

				this->recordBuf = NULL;
				this->recordBufAlloced = 0;
				this->recordBufUsed = 0;

				this->lastTickRotation = VC3(0,0,0);
				this->lastTickPosition = VC3(0,0,0);
				this->lastTickAim = 0;
				this->lastTickAnim = 0;
				this->lastWasMuzzleflash = false;
				this->atTick = 0;
				this->lastWasBlended = false;

				this->yAxisRotation = 0.0f;

				this->skippingToMark[0] = '\0';
				this->skippingToTick = -1;
				this->skipMode = false;

				this->startValuesWritten = false;

				this->scriptProcess = NULL;
			}


			~AniImpl()
			{
			}


			void writeToRecordBuf(const char *string)
			{
				assert(recordBuf != NULL);
				assert(string != NULL);

				int slen = strlen(string);
				if (recordBufUsed + slen >= recordBufAlloced)
				{
					// max alloc limit reached?
					if (recordBufAlloced * 2 > ANI_MAX_RECORD_BUF_ALLOC)
					{
						Logger::getInstance()->warning("AniImpl::writeToRecordBuf - Maximum allocated record buffer size limit reached.");
						Logger::getInstance()->debug(this->name);
						return;
					}
					// recreate the record buffer and make it bigger
					Logger::getInstance()->debug("AniImpl::writeToRecordBuf - Allocated record buffer full, reallocating a bigger buffer.");
					Logger::getInstance()->debug(this->name);

					char *oldBuf = recordBuf;

					recordBufAlloced *= 2;
					recordBuf = new char[recordBufAlloced + 1];

					assert((int)strlen(oldBuf) == recordBufUsed);
					strcpy(recordBuf, oldBuf);

					delete [] oldBuf;

					// WARNING: assuming that now the buffer must be big enough
					// for the string to be appended (all strings smaller than
					// the initial buffer)
				}

				strcpy(&recordBuf[recordBufUsed], string);
				recordBufUsed += slen;

				assert((int)strlen(recordBuf) == recordBufUsed);
			}


			GameScripting *gameScripting;
			Unit *unit;
			char *name;
			bool recording;
			bool playing;

			char *recordBuf;
			int recordBufAlloced;
			int recordBufUsed;

			VC3 lastTickRotation;
			VC3 lastTickPosition;
			float lastTickAim;
			int lastTickAnim;
			int atTick;
			bool lastWasMuzzleflash;
			bool lastWasBlended;

			float yAxisRotation;

			char skippingToMark[ANI_MAX_MARK_LEN + 1];
			int skippingToTick;
			bool skipMode;

			bool startValuesWritten;

			util::ScriptProcess *scriptProcess;

			static char *recordPathName;

			static float globalRotation;
			static VC3 globalOffsetSource;
			static VC3 globalOffsetTarget;

		friend class Ani;
	};


	char *AniImpl::recordPathName = NULL;

	float AniImpl::globalRotation = 0.0f;
	VC3 AniImpl::globalOffsetSource = VC3(0,0,0);
	VC3 AniImpl::globalOffsetTarget = VC3(0,0,0);


	Ani::Ani(GameScripting *gameScripting, Unit *unit)
	{
		if (unit == NULL)
		{
			Logger::getInstance()->error("Ani - Constructor called with null unit parameter (this will be a fatal error).");
		}
		this->impl = new AniImpl(gameScripting, unit);

		unit->setAniRecordBlendFlag(ANIM_NONE);
		unit->setAniRecordBlendEndFlag(false);
		unit->clearAniRecordFireFlag();
	}


	Ani::~Ani()
	{
		delete impl;
		impl = NULL;
	}


	Unit *Ani::getUnit()
	{
		return impl->unit;
	}


	void Ani::setRecordPath(const char *recordPath)
	{
		if (AniImpl::recordPathName != NULL)
		{
			delete [] AniImpl::recordPathName;
			AniImpl::recordPathName = NULL;
		}

		if (recordPath != NULL)
		{
			AniImpl::recordPathName = new char[strlen(recordPath) + 1];
			strcpy(AniImpl::recordPathName, recordPath);
		}
	}


	const char *Ani::getRecordPath()
	{
		return AniImpl::recordPathName;
	}


	// set this ani's name (used by record and play)
	void Ani::setName(const char *aniScriptName)
	{
		if (impl->name != NULL)
		{
			delete [] impl->name;
			impl->name = NULL;
		}

		if (aniScriptName != NULL)
		{
			impl->name = new char[strlen(aniScriptName) + 1];
			strcpy(impl->name, aniScriptName);
		}
	}


	const char *Ani::getName() const
	{
		return impl->name;
	}


	// start recording ani to memory
	void Ani::startRecord(bool appendToOld, int appendAfterTick)
	{
		if (impl->playing || impl->recording)
		{
			Logger::getInstance()->error("Ani::startRecord - Cannot start record, ani already recording or playing.");
			return;
		}
		if (impl->name == NULL)
		{
			Logger::getInstance()->error("Ani::startRecord - Cannot start record, ani has no name set.");
			return;
		}
		impl->recording = true;
		impl->recordBufAlloced = ANI_INITIAL_RECORD_BUF_ALLOC;
		impl->recordBufUsed = 0;
		impl->recordBuf = new char[impl->recordBufAlloced + 1];
		impl->recordBuf[0] = '\0';

		// just created file?
		bool fileWasCreated = false;

		// check that the record file is writable, warn now if it is not.
		// (although it won't yet be written to)
		// also check that the file does not already exist, warn if it does.

		char filepath[256];
		filepath[0] = '\0';

		if (impl->recordPathName != NULL && impl->name != NULL)
		{
			if (strlen(impl->name) + strlen(impl->recordPathName) + 16 < 256)
			{
				strcpy(filepath, impl->recordPathName);
				strcat(filepath, "/ani_");
				strcat(filepath, impl->name);
				strcat(filepath, ".dhs");

				FILE *f = fopen(filepath, "rb");
				if (f != NULL)
				{
					if (!appendToOld)
					{
						Logger::getInstance()->debug("Ani::startRecord - Note, a file with the record name exists, it will be overwritten.");
						Logger::getInstance()->debug("Ani::startRecord - To prevent the file from being overwritten, use cancelRecord to cancel this recording.");
					}
					
					// make a backup...
					fseek(f, 0, SEEK_END);
					int flen = ftell(f);
					fseek(f, 0, SEEK_SET);
					char *backbuf = new char[flen + 1];
					fread(backbuf, flen, 1, f);
					char backuppath[256+64];

					strcpy(backuppath, filepath);
					strcat(backuppath, "_");
					strcat(backuppath, int2str((int)time(0)));
					strcat(backuppath, "_");
					strcat(backuppath, ".bak");
					FILE *fw = fopen(backuppath, "wb");
					if (fw != NULL)
					{
						fwrite(backbuf, flen, 1, fw);
						fclose(fw);
					} else {
						Logger::getInstance()->warning("Ani::startRecord - Failed to create ani backup file.");
					}
					delete [] backbuf;
					// end of backup

					fclose(f);
				} else {
					f = fopen(filepath, "wb");
					if (f != NULL)
					{
						fileWasCreated = true;
						fclose(f);
					} else {
						Logger::getInstance()->error("Ani::startRecord - Note, unable to open file for writing.");
						Logger::getInstance()->error("Ani::startRecord - Fix this problem before stopRecord or the recorded data may be lost.");
					}
				}
			} else {
				Logger::getInstance()->error("Ani::startRecord - Note, file path too long, recorded data will be lost unless the problem is fixed.");
			}
		} else {
			Logger::getInstance()->error("Ani::startRecord - Record path or ani name null.");
		}


		// need to read the beginning of an old ani for appending?
		if (appendToOld && !fileWasCreated)
		{
			assert(filepath[0] != '\0');

			util::SimpleParser sp;
			if (sp.loadFile(filepath))
			{
				int atTick = 0;
				bool insideAni = false;
				while (sp.next())
				{
					char *line = sp.getLine();
					if (strncmp(line, "aniStart", 8) == 0)
					{
						insideAni = true;
					}
					// NEW: Must not confuse this with aniEndBlend - thus expecting space 
					// or end of line immediately after the aniEnd
					if ((strncmp(line, "aniEnd", 6) == 0
						&& (line[6] == ' ' || line[6] == '\r' || line[6] == '\n'))
						|| strncmp(line, "aniWaitUntilAnisEnded", 6) == 0)
					{
						assert(insideAni);
						//impl->atTick = atTick;

						// NEW: pad to requested append position...
						if (appendAfterTick > atTick)
						{
							impl->writeToRecordBuf("// autopadding...\r\n");
							for (int i = atTick; i < appendAfterTick; i++)
							{
								impl->writeToRecordBuf("aniTick\r\n");
							}
						}
						impl->atTick = appendAfterTick;
						break;
					}
					if (strncmp(line, "aniTick", 7) == 0)
					{
						assert(insideAni);
						atTick++;
						if ((atTick % GAME_TICKS_PER_SECOND) == 0)
						{
							impl->writeToRecordBuf("\r\n// ");
							impl->writeToRecordBuf(int2str(atTick / GAME_TICKS_PER_SECOND));
							impl->writeToRecordBuf(" secs \r\n");
						}
						if (appendAfterTick != -1 
							&& atTick > appendAfterTick)
						{
							impl->atTick = atTick;
							break;
						}
					}
					if (insideAni)
					{
						impl->writeToRecordBuf(line);
						impl->writeToRecordBuf("\r\n");
					}
				}
				impl->writeToRecordBuf("\r\n// --- old record ends, last appended record starts ---\r\n\r\n");
			} else {
				Logger::getInstance()->warning("Ani::startRecord - Failed to load previous ani file for appending.");
			}
		} else {
			impl->writeToRecordBuf("aniStart\r\n");
		}
		impl->startValuesWritten = false;

	}


	// saves the recorded ani sequence to disk (with the set name)
	void Ani::stopRecord()
	{
		if (!impl->recording)
		{
			Logger::getInstance()->error("Ani::stopRecord - Cannot stop record, ani is not recording.");
			return;
		}

		impl->writeToRecordBuf("aniWaitUntilAnisEnded\r\naniEnd\r\n");
		//impl->writeToRecordBuf("aniEnd\r\n");

		impl->recordBuf[impl->recordBufUsed] = '\0';

		assert(impl->recordPathName != NULL);
		assert(impl->name != NULL);

		if (impl->recordPathName != NULL && impl->name != NULL)
		{
			char filepath[256];
			if (strlen(impl->name) + strlen(impl->recordPathName) + 16 < 256)
			{
				strcpy(filepath, impl->recordPathName);
				strcat(filepath, "/ani_");
				strcat(filepath, impl->name);
				strcat(filepath, ".dhs");

				FILE *f = fopen(filepath, "wb");
				if (f != NULL)
				{
					const char *tmp = "#!dhs -nopp\r\n";
					fwrite(tmp, strlen(tmp), 1, f);

					tmp = "script ani_";
					fwrite(tmp, strlen(tmp), 1, f);
					tmp = impl->name;
					fwrite(tmp, strlen(tmp), 1, f);
					tmp = "\r\n\r\nsub play_ani\r\n\r\n";
					fwrite(tmp, strlen(tmp), 1, f);

					assert((int)strlen(impl->recordBuf) == impl->recordBufUsed);
					fwrite(impl->recordBuf, impl->recordBufUsed, 1, f);

					tmp = "\r\nendSub\r\n\r\nendScript\r\n\r\n";
					fwrite(tmp, strlen(tmp), 1, f);

					fclose(f);

				} else {
					Logger::getInstance()->error("Ani::stopRecord - Record save failed, file could not be opened for writing.");
				}

			} else {
				Logger::getInstance()->error("Ani::stopRecord - Record save failed, file path too long.");
			}
		} else {
			Logger::getInstance()->error("Ani::stopRecord - Record path or ani name null.");
		}

		//Logger::getInstance()->error(impl->recordBuf);

		// just an easy way to share the record cleanup impl.
		cancelRecord();
	}


	// cancels the recording of the ani sequence (no save)
	void Ani::cancelRecord()
	{
		// notice: this is called by stopRecord too.
		
		if (!impl->recording)
		{
			Logger::getInstance()->error("Ani::stopRecord - Cannot cancel record, ani is not recording.");
			return;
		}

		impl->recording = false;
		assert(impl->recordBuf != NULL);
		delete [] impl->recordBuf;
		impl->recordBuf = NULL;
		impl->recordBufAlloced = 0;
		impl->recordBufUsed = 0;
	}


	// adds the recorded movement, etc. commands automagically.
	// or, plays the aniscript
	void Ani::run()
	{
		if (impl->recording)
		{
			VC3 pos = impl->unit->getPosition();
			VC3 rot = impl->unit->getRotation();

			//if (impl->atTick == 0)
			if (!impl->startValuesWritten)
			{
				impl->startValuesWritten = true;

				// first frame, add warp to location, etc.
				char coords[32];
				char hght[16];
				char rots[64];
				sprintf(coords, "s,%f,%f", pos.x, pos.z);
				sprintf(hght, "%f", pos.y);
				sprintf(rots, "%f,%f,%f", rot.x, rot.y, rot.z);
				impl->writeToRecordBuf("aniWarp ");
				impl->writeToRecordBuf(coords);
				impl->writeToRecordBuf("\r\n");
				impl->writeToRecordBuf("aniHeight ");
				impl->writeToRecordBuf(hght);
				impl->writeToRecordBuf("\r\n");
				impl->writeToRecordBuf("aniRots ");
				impl->writeToRecordBuf(rots);
				impl->writeToRecordBuf("\r\n");
				impl->lastTickPosition = pos;
				impl->lastTickRotation = rot;
			}

			// check for changes in position, etc. and add these to recordbuf...

			// aniMoveX ?
			if (impl->lastTickPosition.x != pos.x)
			{
				float posOffset = pos.x - impl->lastTickPosition.x;
				char offs[32];
				sprintf(offs, "%f", posOffset);
				impl->writeToRecordBuf("aniMoveX ");
				impl->writeToRecordBuf(offs);
				impl->writeToRecordBuf("\r\n");
			}
			// aniMoveZ ?
			if (impl->lastTickPosition.z != pos.z)
			{
				float posOffset = pos.z - impl->lastTickPosition.z;
				char offs[32];
				sprintf(offs, "%f", posOffset);
				impl->writeToRecordBuf("aniMoveZ ");
				impl->writeToRecordBuf(offs);
				impl->writeToRecordBuf("\r\n");
			}
			// aniMoveY ?
			if (impl->lastTickPosition.y != pos.y)
			{
				float posOffset = pos.y - impl->lastTickPosition.y;
				char offs[32];
				sprintf(offs, "%f", posOffset);
				impl->writeToRecordBuf("aniMoveY ");
				impl->writeToRecordBuf(offs);
				impl->writeToRecordBuf("\r\n");
			}
			// aniRotX ?
			if (impl->lastTickRotation.x != rot.x)
			{
				float rotOffset = rot.x - impl->lastTickRotation.x;
				if (rotOffset > 180.0f)
					rotOffset = rotOffset - 360.0f;
				if (rotOffset < -180.0f)
					rotOffset = 360.0f + rotOffset;
				char offs[32];
				sprintf(offs, "%f", rotOffset);
				impl->writeToRecordBuf("aniRotX ");
				impl->writeToRecordBuf(offs);
				impl->writeToRecordBuf("\r\n");
			}
			// aniRotZ ?
			if (impl->lastTickRotation.z != rot.z)
			{
				float rotOffset = rot.z - impl->lastTickRotation.z;
				if (rotOffset > 180.0f)
					rotOffset = rotOffset - 360.0f;
				if (rotOffset < -180.0f)
					rotOffset = 360.0f + rotOffset;
				char offs[32];
				sprintf(offs, "%f", rotOffset);
				impl->writeToRecordBuf("aniRotZ ");
				impl->writeToRecordBuf(offs);
				impl->writeToRecordBuf("\r\n");
			}
			// aniRotY ?
			if (impl->lastTickRotation.y != rot.y)
			{
				float rotOffset = rot.y - impl->lastTickRotation.y;
				if (rotOffset > 180.0f)
					rotOffset = rotOffset - 360.0f;
				if (rotOffset < -180.0f)
					rotOffset = 360.0f + rotOffset;
				char offs[32];
				sprintf(offs, "%f", rotOffset);
				impl->writeToRecordBuf("aniRotY ");
				impl->writeToRecordBuf(offs);
				impl->writeToRecordBuf("\r\n");
			}
			// aniAnim ?
			if (impl->lastTickAnim != impl->unit->getAnimation())
			{
				if (impl->unit->getAnimationSet() != NULL)
				{
					for (int a = 0; a < ANIM_AMOUNT; a++)
					{
						if (impl->unit->getAnimationSet()->getAnimationFileNumber(a) 
							== impl->unit->getAnimation()
							&& (!impl->unit->getAnimationSet()->isAnimationStaticFactored(a)
							|| impl->unit->getAnimationSet()->getAnimationStaticFactor(a)
							== impl->unit->getAnimationSpeedFactor()))
						{
							impl->writeToRecordBuf("aniAnim ");
							impl->writeToRecordBuf(ui::AnimationSet::getAnimName(a));
							//impl->writeToRecordBuf(int2str(impl->unit->getAnimation()));
							impl->writeToRecordBuf("\r\n");
							break;
						}
					}
				}
			}
			// aniAim ?
			//if (impl->lastTickAim != impl->unit->getLastBoneAimDirection())
			if (fabs(impl->lastTickAim - impl->unit->getLastBoneAimDirection()) > 0.5f)
			{
				impl->lastTickAim = impl->unit->getLastBoneAimDirection();

				char aimbuf[32];
				sprintf(aimbuf, "%f", impl->unit->getLastBoneAimDirection());

				impl->writeToRecordBuf("aniAim ");
				impl->writeToRecordBuf(aimbuf);
				impl->writeToRecordBuf("\r\n");
			}

			// aniMuzzleflash ?
			bool muzzleflashNow = (impl->unit->getMuzzleflashVisualEffect() != NULL);
			if (!impl->lastWasMuzzleflash && muzzleflashNow)
			{
				impl->writeToRecordBuf("aniMuzzleflash\r\n");
			}

			// aniEndBlend ?
			if (impl->unit->getAniRecordBlendEndFlag()
				&& impl->lastWasBlended)
			{
				impl->writeToRecordBuf("aniEndBlend\r\n");
				impl->unit->setAniRecordBlendEndFlag(false);
				impl->lastWasBlended = false;
			}

			// aniBlend ?
			if (impl->unit->getAniRecordBlendFlag() != ANIM_NONE)
			{
				impl->writeToRecordBuf("aniBlend ");
				impl->writeToRecordBuf(ui::AnimationSet::getAnimName(impl->unit->getAniRecordBlendFlag()));
				impl->writeToRecordBuf("\r\n");
				impl->unit->setAniRecordBlendFlag(ANIM_NONE);
				impl->lastWasBlended = true;
			}

			// aniFire (Position/Height/Projectile) ?
			if (impl->unit->hasAniRecordFireFlag())
			{
				VC3 pos = impl->unit->getAniRecordFireSourcePosition();
				VC3 targpos = impl->unit->getAniRecordFireDestinationPosition();
				char posbuf[32];

				impl->writeToRecordBuf("aniFirePosition s,");
				sprintf(posbuf, "%f,%f", pos.x, pos.z);
				impl->writeToRecordBuf(posbuf);
				impl->writeToRecordBuf("\r\n");

				impl->writeToRecordBuf("aniFireHeight ");
				sprintf(posbuf, "%f", pos.y);
				impl->writeToRecordBuf(posbuf);
				impl->writeToRecordBuf("\r\n");

				impl->writeToRecordBuf("aniFireTargetPosition s,");
				sprintf(posbuf, "%f,%f", targpos.x, targpos.z);
				impl->writeToRecordBuf(posbuf);
				impl->writeToRecordBuf("\r\n");

				impl->writeToRecordBuf("aniFireTargetHeight ");
				sprintf(posbuf, "%f", targpos.y);
				impl->writeToRecordBuf(posbuf);
				impl->writeToRecordBuf("\r\n");

				Weapon *w = NULL;
				if (impl->unit->getSelectedWeapon() != -1)
				{
					w = impl->unit->getWeaponType(impl->unit->getSelectedWeapon());
				} else {
					for (int i = 0; i < UNIT_MAX_WEAPONS; i++)
					{
						if (impl->unit->isWeaponActive(i))
						{
							w = impl->unit->getWeaponType(i);
							break;
						}
					}
				}
				if (w != NULL)
				{
					impl->writeToRecordBuf("aniFireProjectile ");
					impl->writeToRecordBuf(w->getPartTypeIdString());
					impl->writeToRecordBuf("\r\n");
				}

				impl->unit->clearAniRecordFireFlag();
			}


			impl->atTick++;
			impl->writeToRecordBuf("aniTick");
			//impl->writeToRecordBuf("aniTick ");
			//impl->writeToRecordBuf(int2str(impl->atTick));
			impl->writeToRecordBuf("\r\n");

			if ((impl->atTick % GAME_TICKS_PER_SECOND) == 0)
			{
				impl->writeToRecordBuf("\r\n// ");
				impl->writeToRecordBuf(int2str(impl->atTick / GAME_TICKS_PER_SECOND));
				impl->writeToRecordBuf(" secs \r\n");
			}

			impl->lastTickPosition = pos;
			impl->lastTickRotation = rot;
			impl->lastTickAnim = impl->unit->getAnimation();
			impl->lastWasMuzzleflash = muzzleflashNow;
			// TODO: aim / blend anim

		}

		if (impl->playing)
		{
			if (impl->scriptProcess != NULL)
			{
				impl->gameScripting->runScriptProcess(impl->scriptProcess, true);
				if (impl->scriptProcess->isFinished())
				{
					this->stopPlay();

					// fixes situations where leaping to ani end causes unit obstacles
					// to be left behind to last properly run (non-leaped) position.
					UnitActor *ua = getUnitActorForUnit(impl->unit);
					ua->removeUnitObstacle(impl->unit);
					ua->addUnitObstacle(impl->unit);
				}
			}
		}
	}


	// play the recorded ani sequence (with the set name)
	// (aniscript should have previously been load from disk to memory)
	void Ani::startPlay()
	{
		if (impl->playing || impl->recording)
		{
			Logger::getInstance()->error("Ani::startPlay - Cannot start play, ani already recording or playing.");
			return;
		}
		if (impl->name == NULL)
		{
			Logger::getInstance()->error("Ani::startPlay - Cannot start play, ani has no name set.");
			return;
		}
		impl->playing = true;
		impl->skippingToMark[0] = '\0';
		impl->skippingToTick = -1;
		impl->skipMode = false;
		impl->atTick = 0;

		// TODO: what if there are multiple anis animating the unit???
		// (need to change the animated flag to counter maybe?)
		impl->unit->setAnimated(true);
		impl->unit->setPhysicsObjectFeedbackEnabled(false);

		if (impl->name != NULL)
		{
			char aniScriptName[128];
			aniScriptName[0] = '\0';
			if (strlen(impl->name) < 110)
			{
				strcpy(aniScriptName, "ani_");
				strcat(aniScriptName, impl->name);
			} else {
				Logger::getInstance()->error("Ani::startPlay - Ani name too long.");
				assert(!"Ani::startPlay - Ani name too long.");
			}
			impl->scriptProcess = impl->gameScripting->startUnitScript(impl->unit, aniScriptName, "play_ani");

			if (impl->scriptProcess != NULL)
			{
				util::Script *s = impl->scriptProcess->getScript();
				s->setNoForcedPause(true);
			}

		} else {
			Logger::getInstance()->error("Ani::startPlay - Attempt to start playing ani with no name set.");
			assert(!"Ani::startPlay - Attempt to start playing ani with no name set.");
		}
	}


	// stop the playing of the ani sequence
	// leaving unit to the current position
	void Ani::stopPlay()
	{
		if (!impl->playing)
		{
			Logger::getInstance()->error("Ani::stopRecord - Cannot stop play, ani is not playing.");
			return;
		}
		impl->playing = false;

		// TODO: what if there are multiple anis animating the unit???
		// (need to change the animated flag to counter maybe?)
		impl->unit->setAnimated(false);
		impl->unit->setPhysicsObjectFeedbackEnabled(true);

		if (impl->scriptProcess != NULL)
		{
			impl->gameScripting->deleteGameScript(impl->scriptProcess);
			impl->scriptProcess = NULL;
		}
	}


	// true/false if the play has ended
	bool Ani::hasPlayEnded() const
	{
		return !impl->playing;
	}


	// skip beginning of the ani until given ticks reached
	// (current unit state will not altered)
	void Ani::skipToPosition(int ticksFromStart)
	{
		impl->skippingToTick = ticksFromStart;
		impl->skipMode = true;
	}


	// immediate play beginning of the ani until given ticks reached
	// (unit state will be set to properly to that moment)
	void Ani::leapToPosition(int ticksFromStart)
	{
		impl->skippingToTick = ticksFromStart;
		impl->skipMode = false;
	}


	// skip beginning of the ani until given mark reached
	// (current unit state will not altered)
	void Ani::skipToMark(const char *mark)
	{
		if (mark != NULL
			&& strlen(mark) < ANI_MAX_MARK_LEN
			&& mark[0] != '\0')
		{
			strcpy(impl->skippingToMark, mark);
			impl->skipMode = true;
		} else {
			Logger::getInstance()->error("Ani::skipToMark - Cannot skip to given mark, given mark name too long, empty or null.");
		}
	}


	// immediate play beginning of the ani until given mark reached
	// (unit state will be set to properly to that moment)
	void Ani::leapToMark(const char *mark)
	{
		if (mark != NULL
			&& strlen(mark) < ANI_MAX_MARK_LEN
			&& mark[0] != '\0')
		{
			strcpy(impl->skippingToMark, mark);
			impl->skipMode = false;
		} else {
			Logger::getInstance()->error("Ani::skipToMark - Cannot skip to given mark, given mark name too long, empty or null.");
		}
	}


	// add a named marker
	void Ani::addMark(const char *markName)
	{
		if (markName == NULL)
		{
			assert(!"Ani::addMark - null mark name parameter given.");
			return;
		}

		if (!impl->recording)
		{
			Logger::getInstance()->error("Ani::addMark - Cannot add a mark, ani not recording.");
			return;
		}
		impl->writeToRecordBuf("mark ");
		impl->writeToRecordBuf(markName);
		impl->writeToRecordBuf("\n");
	}


	// for scripting system's use (others may find these a bit
	// useless ;)
	// return true if the system should pause
	bool Ani::reachedMark(const char *markName)
	{
		assert(markName != NULL);
		if (impl->skippingToMark[0] != '\0')
		{
			if (strcmp(markName, impl->skippingToMark) == 0)
			{
				impl->skippingToMark[0] = '\0';
				return true;
			}
		}
		return false;
	}

	
	bool Ani::reachedTick()
	{
		impl->atTick++;
		if (impl->skippingToTick != -1)
		{
			if (impl->atTick >= impl->skippingToTick)
			{
				impl->skippingToTick = -1;
				return true;
			} else {
				return false;
			}
		} else {
			return true;
		}
	}


	void Ani::aniStart()
	{
		impl->unit->setAnimated(true);
		impl->unit->setPhysicsObjectFeedbackEnabled(false);
	}

  
	void Ani::aniEnd()
	{
		impl->unit->setAnimated(false);
		impl->unit->setPhysicsObjectFeedbackEnabled(true);

		impl->unit->setPath(NULL);
		impl->unit->setWaypoint(impl->unit->getPosition());
		impl->unit->setFinalDestination(impl->unit->getPosition());
	}


	void Ani::aniWarp(float x, float z)
	{
		VC3 pos = impl->unit->getPosition();
		pos.x = x;
		pos.z = z;

		// new: global offsetting via 2 pivots and rotation around the nwe "pivot".
		pos -= AniImpl::globalOffsetSource;

		float tmpX = pos.x;
		pos.x = pos.x * cosf(UNIT_ANGLE_TO_RAD(AniImpl::globalRotation)) + pos.z * sinf(UNIT_ANGLE_TO_RAD(AniImpl::globalRotation));
		pos.z = pos.z * cosf(UNIT_ANGLE_TO_RAD(AniImpl::globalRotation)) - tmpX * sinf(UNIT_ANGLE_TO_RAD(AniImpl::globalRotation));

		pos += AniImpl::globalOffsetTarget;

		impl->unit->setPosition(pos);
	}

	
	void Ani::aniHeight(float height)
	{
		VC3 pos = impl->unit->getPosition();
		pos.y = height;

		// new: global offsetting via 2 pivots.
		pos.y -= AniImpl::globalOffsetSource.y;
		pos.y += AniImpl::globalOffsetTarget.y;

		impl->unit->setPosition(pos);
	}

	
	void Ani::aniRots(float angleX, float angleY, float angleZ)
	{
		if (impl->yAxisRotation + impl->globalRotation != 0.0f)
		{
			float newY = angleY + impl->yAxisRotation + impl->globalRotation;
			while (newY >= 360.0f) newY -= 360.0f;
			while (newY < 0.0f) newY += 360.0f;			
			impl->unit->setRotation(angleX, newY, angleZ);
		} else {
			impl->unit->setRotation(angleX, angleY, angleZ);
		}
	}

	
	void Ani::aniAnim(int anim)
	{
		// TODO: this ok?
		ui::AnimationSet *animset = impl->unit->getAnimationSet();
		float timefact = impl->unit->getCustomTimeFactor();
		if (animset != NULL)
		{
			if (animset->isAnimationInSet(anim))
			{
				animset->animate(impl->unit, anim);
				if (animset->isAnimationStaticFactored(anim))
				{
					timefact *= animset->getAnimationStaticFactor(anim);
				}
				ui::Animator::setAnimationSpeedFactor(impl->unit, timefact);
			}
		}
		//ui::Animator::setAnimation(impl->unit, anim); 
	}

	
	void Ani::aniEndAnim(int anim)
	{
		// TODO: umm.. how exactly is this done...??
		// probably need to ask AnimationSet about the blend number and 
		// then use the Animator class directly to end the blend anim...

		ui::AnimationSet *animset = impl->unit->getAnimationSet();
		if (animset != NULL)
		{
			if (animset->isAnimationInSet(anim))
			{
				if (animset->getAnimationBlendNumber(anim) != 0)
				{
					ui::Animator::endBlendAnimation(impl->unit, animset->getAnimationBlendNumber(anim) - 1, true);
				} else {
					Logger::getInstance()->warning("Ani::aniEndAnim - Ending a base (non-blended) animation. Something probably went wrong?");
					// TODO: should still end the base animation?
				}
			}
		}
	}

	
	void Ani::aniMoveX(float offsetX)
	{
		VC3 pos = impl->unit->getPosition();
		if (impl->yAxisRotation + impl->globalRotation != 0.0f)
		{
			pos.x += offsetX * cosf(UNIT_ANGLE_TO_RAD(impl->yAxisRotation + impl->globalRotation));
			pos.z -= offsetX * sinf(UNIT_ANGLE_TO_RAD(impl->yAxisRotation + impl->globalRotation));
		} else {
			pos.x += offsetX;
		}
		impl->unit->setPosition(pos);
	}

	
	void Ani::aniMoveY(float offsetY)
	{
		VC3 pos = impl->unit->getPosition();
		pos.y += offsetY;
		impl->unit->setPosition(pos);
	}

	
	void Ani::aniOnGround(float height)
	{
		VC3 pos = impl->unit->getPosition();
		pos.y = height;
		impl->unit->setPosition(pos);
	}

	
	void Ani::aniMoveZ(float offsetZ)
	{
		VC3 pos = impl->unit->getPosition();
		if (impl->yAxisRotation + impl->globalRotation != 0.0f)
		{
			pos.x += offsetZ * sinf(UNIT_ANGLE_TO_RAD(impl->yAxisRotation + impl->globalRotation));
			pos.z += offsetZ * cosf(UNIT_ANGLE_TO_RAD(impl->yAxisRotation + impl->globalRotation));
		} else {
			pos.z += offsetZ;
		}
		impl->unit->setPosition(pos);
	}

	
	void Ani::aniRotX(float rotateX)
	{
		VC3 rot = impl->unit->getRotation();
		rot.x += rotateX;
		if (rot.x < 0)
			rot.x += 360.0f;
		if (rot.x >= 360)
			rot.x -= 360.0f;
		impl->unit->setRotation(rot.x, rot.y, rot.z);
	}


	void Ani::aniRotY(float rotateY)
	{
		VC3 rot = impl->unit->getRotation();
		rot.y += rotateY;
		if (rot.y < 0)
			rot.y += 360.0f;
		if (rot.y >= 360)
			rot.y -= 360.0f;
		impl->unit->setRotation(rot.x, rot.y, rot.z);
	}

	
	void Ani::aniRotZ(float rotateZ)
	{
		VC3 rot = impl->unit->getRotation();
		rot.z += rotateZ;
		if (rot.z < 0)
			rot.z += 360.0f;
		if (rot.z >= 360)
			rot.z -= 360.0f;
		impl->unit->setRotation(rot.x, rot.y, rot.z);
	}

	
	void Ani::aniAim(float aimY)
	{
		// HACK: set target to get proper aiming direction...
		/*
		VC3 pos = impl->unit->getPosition();
		pos.x += -10.0f * sinf(UNIT_ANGLE_TO_RAD(aimY));
		pos.z += -10.0f * cosf(UNIT_ANGLE_TO_RAD(aimY));
		impl->unit->targeting.setTarget(pos);
		impl->unit->targeting.setAimingPosition(pos);				
		*/

		impl->unit->setLastBoneAimDirection(aimY);
		if (impl->unit->getVisualObject() != NULL)
		{
			impl->unit->getVisualObject()->rotateBone(impl->unit->getUnitType()->getAimBone(), aimY, 0);
		}
	}

	
	void Ani::aniEndAim()
	{
		// this ok?
		impl->unit->setLastBoneAimDirection(0);
		if (impl->unit->getVisualObject() != NULL)
		{
			//impl->unit->getVisualObject()->rotateBone(impl->unit->getUnitType()->getAimBone(), 0, 0);
			impl->unit->getVisualObject()->releaseRotatedBone(impl->unit->getUnitType()->getAimBone());
		}
	}


	void Ani::aniAxis(float yAxisRotation)
	{
		impl->yAxisRotation = yAxisRotation;
	}


	void Ani::aniIgnore(const char *ignoreOperation)
	{
		if (ignoreOperation == NULL)
		{
			assert(!"Ani::aniIgnore - null parameter");
			return;
		}

		if (strcmp(ignoreOperation, "aim") == 0)
		{
			// TODO
		}
		else if (strcmp(ignoreOperation, "anim") == 0)
		{
			// TODO
		}
		else if (strcmp(ignoreOperation, "pos") == 0)
		{
			// TODO
		}
		else if (strcmp(ignoreOperation, "roty") == 0)
		{
			// TODO
		}
		else if (strcmp(ignoreOperation, "rotx") == 0)
		{
			// TODO
		}
		else if (strcmp(ignoreOperation, "rotz") == 0)
		{
			// TODO
		}
	}

	bool Ani::aniWaitUntilAnisEnded()
	{
		if (impl->skippingToTick < ANI_MAX_TICKS)
		{ 
			return true;
		}
		return false;
	}

	void Ani::leapToEnd()
	{
		this->leapToPosition(ANI_MAX_TICKS);
		if (impl->scriptProcess != NULL)
		{
			// WARNING: unsafe cast
			GameScriptData *gsd = (GameScriptData *)impl->scriptProcess->getData();
			if (gsd != NULL)
			{
				gsd->waitCounter = 0;
				gsd->waitDestination = false;
			}
		}
	}

	void Ani::aniEndBlend()
	{
		ui::Animator::endBlendAnimation(impl->unit, 0, true); 
	}

	void Ani::aniBlend(int anim)
	{
		if (impl->skippingToTick != -1)
		{
			if (impl->atTick < impl->skippingToTick)
			{
				return;
			}
		}

		ui::AnimationSet *animset = impl->unit->getAnimationSet();
		if (animset != NULL)
		{
			if (animset->isAnimationInSet(anim))
			{
				int animfile = animset->getAnimationFileNumber(anim);
				// NOTE: blending-in disabled, as that would interfere with shooting anims (that have to be instant)
				//ui::Animator::setBlendAnimation(impl->unit, 0, animfile, true, true); 
				ui::Animator::setBlendAnimation(impl->unit, 0, animfile, false, true); 
			}
		}
	}

	void Ani::aniMuzzleflash()
	{
		if (impl->skippingToTick != -1)
		{
			if (impl->atTick < impl->skippingToTick)
			{
				return;
			}
		}

		if (impl->unit->getSelectedWeapon() != -1)
		{
			UnitActor *ua = getUnitActorForUnit(impl->unit);
			ua->createMuzzleflash(impl->unit, impl->unit->getSelectedWeapon());
		}
	}


	void Ani::aniFireProjectile(Game *game, Weapon *weap, const VC3 &projpos, const VC3 &projtarg)
	{
		if (impl->skippingToTick != -1)
		{
			if (impl->atTick < impl->skippingToTick)
			{
				return;
			}
		}

		VC3 dir = projtarg - projpos;
		if (dir.GetSquareLength() > 0.001f)
		{
			dir.Normalize();
		} else {
			dir = VC3(0,1,0);
		}
		ParticleSpawner::spawnProjectileWithWeapon(game, weap, projpos, dir);
	}

	void Ani::setGlobalOffsetSource(const VC3 &sourcePosition)
	{
		AniImpl::globalOffsetSource = sourcePosition;
	}

	void Ani::setGlobalOffsetTarget(const VC3 &targetPosition)
	{
		AniImpl::globalOffsetTarget = targetPosition;
	}

	void Ani::setGlobalRotation(float rotationY)
	{
		AniImpl::globalRotation = rotationY;
	}

	void Ani::resetGlobalOffsetAndRotation()
	{
		AniImpl::globalRotation = 0.0f;
		AniImpl::globalOffsetSource = VC3(0,0,0);
		AniImpl::globalOffsetTarget = VC3(0,0,0);
	}

	VC3 Ani::getGlobalOffsetSource()
	{
		return AniImpl::globalOffsetSource;
	}

	VC3 Ani::getGlobalOffsetTarget()
	{
		return AniImpl::globalOffsetTarget;
	}

	float Ani::getGlobalRotation()
	{
		return AniImpl::globalRotation;
	}

}


