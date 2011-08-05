
#include "precompiled.h"

#include "AniTool.h"

#include "../game/GameMap.h"
#include "../system/Logger.h"
#include "../util/TextFinder.h"
#include "../util/fb_assert.h"
#include "../convert/str2int.h"
#include <DatatypeDef.h>
#include <string>
#include <boost/lexical_cast.hpp>

namespace game
{
	class AniToolImpl
	{
		private:
			AniToolImpl()
			{
				this->bufstr = "";
				this->filename = "";
				this->totalTicks = 0;
				this->startPosition = 0;
				this->endPosition = 0;
			}

			~AniToolImpl()
			{
			}

			void keepSelectionsInsideBuffer()
			{
				if (startPosition < 0)
					startPosition = 0;
				if (endPosition < 0)
					endPosition = 0;
				if (startPosition > (int)bufstr.size())
					startPosition = (int)bufstr.size();
				if (endPosition > (int)bufstr.size())
					endPosition = (int)bufstr.size();

				if (endPosition < startPosition)
					endPosition = startPosition;
			}

			void calcTotalTicks()
			{
				totalTicks = 0;
				util::TextFinder tf(bufstr.c_str(), true);
				totalTicks = tf.countOccurances("aniTick");
			}

			int getTickPosition(int tick)
			{
				util::TextFinder tf(bufstr.c_str(), true);
				if (tick == 0)
				{
					tf.findNext("aniStart");
					int startpos = tf.findNext("ani");
					if (startpos >= 0)
					{
						return startpos;
					} else {
						Logger::getInstance()->error("AniToolImpl::getTickPosition - Failed to find aniStart.");
						return 0;
					}
				}
				return tf.findOneOfMany("aniTick", tick);
			}

			int getTickCommandEndPosition(int position)
			{
				if (position == 0)
				{
					return 0;
				}
			}

			void getParsedDataBuffers(bool **selected_out, VC3 **positions_out, VC3 **rotations_out, float **aims_out)
			{
				bool *selected = new bool[totalTicks];
				VC3 *positions = new VC3[totalTicks];
				VC3 *rotations = new VC3[totalTicks];
				float *aims = new float[totalTicks];

				if (selected_out != NULL)
				{
					*selected_out = selected;
				}
				if (positions_out != NULL)
				{
					*positions_out = positions;
				}
				if (rotations_out != NULL)
				{
					*rotations_out = rotations;
				}
				if (aims_out != NULL)
				{
					*aims_out = aims;
				}

				util::TextFinder tf(bufstr.c_str(), true);

				positions[0] = VC3(0,0,0);
				rotations[0] = VC3(0,0,0);
				aims[0] = 0;
				selected[0] = false;

				// read in absolute start position
				{
					int aniWarpPos = tf.findNext("aniWarp");
					if (aniWarpPos != -1)
					{
						int aniWarpComma1 = tf.findNext(",");
						int aniWarpComma2 = tf.findNext(",");
						if (aniWarpComma1 != -1
							&& aniWarpComma2 != -1)
						{
							positions[0].x = (float)atof(&(bufstr.c_str()[aniWarpComma1 + 1]));
							positions[0].z = (float)atof(&(bufstr.c_str()[aniWarpComma2 + 1]));
						}
					}
					tf.moveToStart();
				}

				// read in aniHeight
				{
					int aniHeightPos = tf.findNext("aniHeight");
					positions[0].y = (float)atof(&(bufstr.c_str()[aniHeightPos + 9]));
					tf.moveToStart();
				}

				int lasttickpos = tf.findNext("aniStart");

				for (int i = 1; i < totalTicks; i++)
				{
					int nexttickpos = 0;

					tf.moveToPosition(lasttickpos);

					nexttickpos = tf.findNext("aniTick");
					if (nexttickpos != -1
						&& lasttickpos != -1)
					{
						if (lasttickpos >= startPosition
							&& nexttickpos <= endPosition)
						{
							selected[i] = true;
						} else {
							selected[i] = false;
						}

						tf.moveToPosition(lasttickpos);
						int moveXpos = tf.findNext("aniMoveX");
						tf.moveToPosition(lasttickpos);
						int moveYpos = tf.findNext("aniMoveY");
						tf.moveToPosition(lasttickpos);
						int moveZpos = tf.findNext("aniMoveZ");
						tf.moveToPosition(lasttickpos);
						int rotXpos = tf.findNext("aniRotX");
						tf.moveToPosition(lasttickpos);
						int rotYpos = tf.findNext("aniRotY");
						tf.moveToPosition(lasttickpos);
						int rotZpos = tf.findNext("aniRotZ");
						tf.moveToPosition(lasttickpos);
						int aimpos = tf.findNext("aniAim");

						if (moveXpos >= 0 && moveXpos < nexttickpos)
						{
							positions[i].x = (float)atof(&(bufstr.c_str()[moveXpos + 8]));
							positions[i].x += positions[i - 1].x;
						} else {
							positions[i].x = positions[i - 1].x;
							//positions[i].x = 0;
						}
						if (moveYpos >= 0 && moveYpos < nexttickpos)
						{
							positions[i].y = (float)atof(&(bufstr.c_str()[moveYpos + 8]));
							positions[i].y += positions[i - 1].y;
						} else {
							positions[i].y = positions[i - 1].y;
							//positions[i].y = 0;
						}
						if (moveZpos >= 0 && moveZpos < nexttickpos)
						{
							positions[i].z = (float)atof(&(bufstr.c_str()[moveZpos + 8]));
							positions[i].z += positions[i - 1].z;
						} else {
							positions[i].z = positions[i - 1].z;
							//positions[i].z = 0;
						}

						if (rotXpos >= 0 && rotXpos < nexttickpos)
						{
							rotations[i].x = (float)atof(&(bufstr.c_str()[rotXpos + 7]));
							rotations[i].x += rotations[i - 1].x;
						} else {
							rotations[i].x = rotations[i - 1].x;
						}
						if (rotYpos >= 0 && rotYpos < nexttickpos)
						{
							rotations[i].y = (float)atof(&(bufstr.c_str()[rotYpos + 7]));
							rotations[i].y += rotations[i - 1].y;
						} else {
							rotations[i].y = rotations[i - 1].y;
						}
						if (rotZpos >= 0 && rotZpos < nexttickpos)
						{
							rotations[i].z = (float)atof(&(bufstr.c_str()[rotZpos + 7]));
							rotations[i].z += rotations[i - 1].z;
						} else {
							rotations[i].z = rotations[i - 1].z;
						}

						if (aimpos >= 0 && aimpos < nexttickpos)
						{
							aims[i] = (float)atof(&(bufstr.c_str()[aimpos + 6]));
							// unlike rotations, this is not relative!
							//aims[i] += aims[i - 1];
						} else {
							aims[i] = aims[i - 1];
						}

					} else {
						//positions[i] = VC3(0,0,0);
						positions[i] = positions[i - 1];
						rotations[i] = rotations[i - 1];
						aims[i] = aims[i - 1];
						selected[i] = false;
					}
					lasttickpos = nexttickpos;				
				}

				if (selected_out == NULL)
				{
					delete [] selected;
				}
				if (positions_out == NULL)
				{
					delete [] positions;
				}
				if (rotations_out == NULL)
				{
					delete [] rotations;
				}
				if (aims_out == NULL)
				{
					delete [] aims;
				}
			}

			void applyDataBuffers(VC3 *positions, VC3 *rotations, float *aims)
			{
				util::TextFinder *tf = new util::TextFinder(bufstr.c_str(), true);

				// fix aniheight (required if dropOnGround called)
				if (positions != NULL)
				{
					int aniHeightPos = tf->findNext("aniHeight");
					if (aniHeightPos != -1)
					{
						int aniHeightPosEnd = tf->findNext("\n") + 1;
						if (aniHeightPosEnd > 0)
						{
							std::string newheight = std::string("aniHeight ") + boost::lexical_cast<std::string> (positions[0].y);
							newheight += std::string("\r\n");
							bufstr.replace(aniHeightPos, aniHeightPosEnd - aniHeightPos, newheight.c_str());

							delete tf;
							tf = new util::TextFinder(bufstr.c_str(), true);
						}
					}
					tf->moveToStart();
				}

				int lasttickpos = tf->findNext("aniStart");

				for (int i = 1; i < totalTicks; i++)
				{
					int nexttickpos = 0;

					tf->moveToPosition(lasttickpos);

					nexttickpos = tf->findNext("aniTick");
					if (nexttickpos != -1
						&& lasttickpos != -1
						&& lasttickpos >= startPosition && lasttickpos < endPosition)
					{
						// move X ---

						if (positions != NULL)
						{
							tf->moveToPosition(lasttickpos);
							int moveXpos = tf->findNext("aniMoveX");
							int moveXposEnd = tf->findNext("\n") + 1;

							std::string newmove = "";
							if (fabs(positions[i].x - positions[i - 1].x) > 0.0001f)
							{
								newmove = std::string("aniMoveX ") + boost::lexical_cast<std::string> (positions[i].x - positions[i - 1].x);
								newmove += std::string("\r\n");
							}
							if (moveXpos >= 0 && moveXpos < nexttickpos
								&& moveXposEnd > 0 && moveXposEnd < nexttickpos+1)
							{
								bufstr.replace(moveXpos, moveXposEnd - moveXpos, newmove.c_str());
							} else {
								bufstr.replace(nexttickpos, 0, newmove.c_str());
							}

							delete tf;
							tf = new util::TextFinder(bufstr.c_str(), true);
							tf->moveToPosition(lasttickpos);
							nexttickpos = tf->findNext("aniTick");
						}

						// move Y ---

						if (positions != NULL)
						{
							tf->moveToPosition(lasttickpos);
							int moveYpos = tf->findNext("aniMoveY");
							int moveYposEnd = tf->findNext("\n") + 1;

							std::string newmove = "";
							if (fabs(positions[i].y - positions[i - 1].y) > 0.0001f)
							{
								newmove = std::string("aniMoveY ") + boost::lexical_cast<std::string> (positions[i].y - positions[i - 1].y);
								newmove += std::string("\r\n");
							}
							if (moveYpos >= 0 && moveYpos < nexttickpos
								&& moveYposEnd > 0 && moveYposEnd < nexttickpos+1)
							{
								bufstr.replace(moveYpos, moveYposEnd - moveYpos, newmove.c_str());
							} else {
								bufstr.replace(nexttickpos, 0, newmove.c_str());
							}

							delete tf;
							tf = new util::TextFinder(bufstr.c_str(), true);
							tf->moveToPosition(lasttickpos);
							nexttickpos = tf->findNext("aniTick");
						}

						// move Z ---

						if (positions != NULL)
						{
							tf->moveToPosition(lasttickpos);
							int moveZpos = tf->findNext("aniMoveZ");
							int moveZposEnd = tf->findNext("\n") + 1;

							std::string newmove = "";
							if (fabs(positions[i].z - positions[i - 1].z) > 0.0001f)
							{
								newmove = std::string("aniMoveZ ") + boost::lexical_cast<std::string> (positions[i].z - positions[i - 1].z);
								newmove += std::string("\r\n");
							}
							if (moveZpos >= 0 && moveZpos < nexttickpos
								&& moveZposEnd > 0 && moveZposEnd < nexttickpos+1)
							{
								bufstr.replace(moveZpos, moveZposEnd - moveZpos, newmove.c_str());
							} else {
								bufstr.replace(nexttickpos, 0, newmove.c_str());
							}

							delete tf;
							tf = new util::TextFinder(bufstr.c_str(), true);
							tf->moveToPosition(lasttickpos);
							nexttickpos = tf->findNext("aniTick");
						}

						// rotate X ---

						if (rotations != NULL)
						{
							tf->moveToPosition(lasttickpos);
							int rotXpos = tf->findNext("aniRotX");
							int rotXposEnd = tf->findNext("\n") + 1;

							std::string newrot = "";
							if (fabs(rotations[i].x - rotations[i - 1].x) > 0.0001f)
							{
								newrot = std::string("aniRotX ") + boost::lexical_cast<std::string> (rotations[i].x - rotations[i - 1].x);
								newrot += std::string("\r\n");
							}
							if (rotXpos >= 0 && rotXpos < nexttickpos
								&& rotXposEnd > 0 && rotXposEnd < nexttickpos+1)
							{
								bufstr.replace(rotXpos, rotXposEnd - rotXpos, newrot.c_str());
							} else {
								bufstr.replace(nexttickpos, 0, newrot.c_str());
							}

							delete tf;
							tf = new util::TextFinder(bufstr.c_str(), true);
							tf->moveToPosition(lasttickpos);
							nexttickpos = tf->findNext("aniTick");
						}

						// rotate Y ---

						if (rotations != NULL)
						{
							tf->moveToPosition(lasttickpos);
							int rotYpos = tf->findNext("aniRotY");
							int rotYposEnd = tf->findNext("\n") + 1;

							std::string newrot = "";
							if (fabs(rotations[i].y - rotations[i - 1].y) > 0.0001f)
							{
								newrot = std::string("aniRotY ") + boost::lexical_cast<std::string> (rotations[i].y - rotations[i - 1].y);
								newrot += std::string("\r\n");
							}
							if (rotYpos >= 0 && rotYpos < nexttickpos
								&& rotYposEnd > 0 && rotYposEnd < nexttickpos+1)
							{
								bufstr.replace(rotYpos, rotYposEnd - rotYpos, newrot.c_str());
							} else {
								bufstr.replace(nexttickpos, 0, newrot.c_str());
							}

							delete tf;
							tf = new util::TextFinder(bufstr.c_str(), true);
							tf->moveToPosition(lasttickpos);
							nexttickpos = tf->findNext("aniTick");
						}

						// rotate Z ---

						if (rotations != NULL)
						{
							tf->moveToPosition(lasttickpos);
							int rotZpos = tf->findNext("aniRotZ");
							int rotZposEnd = tf->findNext("\n") + 1;

							std::string newrot = "";
							if (fabs(rotations[i].z - rotations[i - 1].z) > 0.0001f)
							{
								newrot = std::string("aniRotZ ") + boost::lexical_cast<std::string> (rotations[i].z - rotations[i - 1].z);
								newrot += std::string("\r\n");
							}
							if (rotZpos >= 0 && rotZpos < nexttickpos
								&& rotZposEnd > 0 && rotZposEnd < nexttickpos+1)
							{
								bufstr.replace(rotZpos, rotZposEnd - rotZpos, newrot.c_str());
							} else {
								bufstr.replace(nexttickpos, 0, newrot.c_str());
							}

							delete tf;
							tf = new util::TextFinder(bufstr.c_str(), true);
							tf->moveToPosition(lasttickpos);
							nexttickpos = tf->findNext("aniTick");
						}

						// aim ---

						if (aims != NULL)
						{
							tf->moveToPosition(lasttickpos);
							int aimpos = tf->findNext("aniAim");
							int aimposEnd = tf->findNext("\n") + 1;

							std::string newrot = "";
							if (fabs(aims[i] - aims[i - 1]) > 0.01f)
							{
								newrot = std::string("aniAim ") + boost::lexical_cast<std::string> (aims[i]);
								newrot += std::string("\r\n");
							}
							if (aimpos >= 0 && aimpos < nexttickpos
								&& aimposEnd > 0 && aimposEnd < nexttickpos+1)
							{
								bufstr.replace(aimpos, aimposEnd - aimpos, newrot.c_str());
							} else {
								bufstr.replace(nexttickpos, 0, newrot.c_str());
							}

							delete tf;
							tf = new util::TextFinder(bufstr.c_str(), true);
							tf->moveToPosition(lasttickpos);
							nexttickpos = tf->findNext("aniTick");
						}
					}
					lasttickpos = nexttickpos;				
				}
			}


			int startPosition;
			int endPosition;
			int totalTicks;
			std::string bufstr;
			std::string filename;

		friend class AniTool;
	};


	AniTool::AniTool()
	{
		this->impl = new AniToolImpl();
	}

	AniTool::~AniTool()
	{
		delete this->impl;
	}

	bool AniTool::loadFile(const char *filename)
	{
		FILE *f = fopen(filename, "rb");
		if (f != NULL)
		{
			fseek(f, 0, SEEK_END);
			int flen = ftell(f);
			fseek(f, 0, SEEK_SET);

			char *tmp = new char[flen + 1];

			int got = fread(tmp, flen, 1, f);
			if (got != 1)
			{
				delete [] tmp;
				fclose(f);
				impl->filename = "";
				impl->totalTicks = 0;
				impl->startPosition = 0;
				impl->endPosition = 0;
				return false;
			}

			tmp[flen] = '\0';
			impl->bufstr = tmp;

			delete [] tmp;

			fclose(f);
			impl->filename = filename;
			impl->calcTotalTicks();
			impl->startPosition = 0;
			impl->endPosition = 0;

			return true;
		} else {
			impl->filename = "";
			impl->totalTicks = 0;
			impl->startPosition = 0;
			impl->endPosition = 0;
			return false;
		}
	}

	void AniTool::saveFile()
	{
		if (!impl->filename.empty())
		{
			saveFileAs(impl->filename.c_str());
		} else {
			Logger::getInstance()->error("AniTool::saveFile() - Cannot save, no filename given (no file loaded).");
		}
	}

	void AniTool::saveFileAs(const char *filename)
	{
		fb_assert(filename != NULL);

		FILE *f = fopen(filename, "wb");
		if (f != NULL)
		{
			int got = fwrite(impl->bufstr.c_str(), impl->bufstr.size(), 1, f);
			if (got != 1)
			{
				Logger::getInstance()->error("AniTool::saveFileAs - Error while writing data to file.");
			}
			fclose(f);
		} else {
			Logger::getInstance()->error("AniTool::saveFileAs - Failed to open file for writing.");
		}
	}

	void AniTool::close()
	{
		impl->filename = "";
		impl->bufstr = "";
		impl->startPosition = 0;
		impl->endPosition = 0;
	}

	void AniTool::setSelectionStart(int tickPosition)
	{
		impl->startPosition = impl->getTickPosition(tickPosition);
		if (impl->startPosition < 0)
			impl->startPosition = impl->bufstr.size();
		impl->keepSelectionsInsideBuffer();
	}

	void AniTool::setSelectionEnd(int tickPosition)
	{
		impl->endPosition = impl->getTickPosition(tickPosition);
		if (impl->endPosition < 0)
			impl->endPosition = impl->bufstr.size();
		impl->keepSelectionsInsideBuffer();
	}

	void AniTool::setSelectionStartToStart()
	{
		impl->startPosition = 0;
	}

	void AniTool::setSelectionEndToEnd()
	{
		impl->endPosition = impl->bufstr.size();
	}

	void AniTool::padTicksUntil(int tickPosition)
	{
		if (tickPosition <= impl->totalTicks)
		{
			return;
		}
		impl->bufstr += "// padded ticks";
		for (int i = 0; i < tickPosition - impl->totalTicks; i++)
		{
			impl->bufstr += "aniTick\r\n";
		}
		impl->totalTicks = tickPosition;
	}

	void AniTool::offsetFirstWarp(float offsetX, float offsetZ)
	{
		// TODO
	}

	void AniTool::tweakMovementToward(float offsetX, float offsetZ, bool smooth)
	{
		bool *selected = NULL;
		VC3 *positions = NULL;

		impl->getParsedDataBuffers(&selected, &positions, NULL, NULL);

		VC3 *positionsNew = new VC3[impl->totalTicks];

		for (int init = 0; init < impl->totalTicks; init++)
		{
			positionsNew[init] = positions[init];
		}

		int tweakLen = impl->endPosition - impl->startPosition;

		for (int i = 0; i < impl->totalTicks; i++)
		{
			if (selected[i])
			{
				positionsNew[i] = positions[i];
				float factor = (float)(i - impl->startPosition) / (float)tweakLen;
				if (smooth)
				{
					factor = (float)sinf(3.1415926f * (factor - 0.5f)) * 0.5f + 0.5f;
				}
				positionsNew[i].x += offsetX * factor;
				positionsNew[i].z += offsetZ * factor;
			}
		}

		impl->applyDataBuffers(positionsNew, NULL, NULL);

		delete [] selected;
		delete [] positionsNew;
		delete [] positions;		
	}

	void AniTool::smoothMovement(int smoothAmount)
	{
		bool *selected = NULL;
		VC3 *positions = NULL;

		impl->getParsedDataBuffers(&selected, &positions, NULL, NULL);

		VC3 *positionsNew = new VC3[impl->totalTicks];

		for (int init = 0; init < impl->totalTicks; init++)
		{
			positionsNew[init] = positions[init];
		}

		for (int i = 0; i < impl->totalTicks; i++)
		{
			if (selected[i])
			{
				int mini = i - smoothAmount;
				if (mini < 0)
					mini = 0;
				int maxi = i + smoothAmount;
				if (maxi > impl->totalTicks - 1)
					maxi = impl->totalTicks - 1;
				
				positionsNew[i] = VC3(0,0,0);
				if (mini < maxi)
				{
					float totalWeight = 0;
					for (int j = mini; j < maxi; j++)
					{
						float weight = (float)smoothAmount - (float)fabs(float(j - i));
						positionsNew[i] += positions[j] * weight;
						totalWeight += weight;
					}
					positionsNew[i] /= totalWeight;
				} else {
					positionsNew[i] = positions[i];
				}
			}
		}

		impl->applyDataBuffers(positionsNew, NULL, NULL);

		delete [] selected;
		delete [] positionsNew;
		delete [] positions;		
	}

	void AniTool::smoothRotation(int smoothAmount)
	{
		bool *selected = NULL;
		VC3 *rotations = NULL;

		impl->getParsedDataBuffers(&selected, NULL, &rotations, NULL);

		VC3 *rotationsNew = new VC3[impl->totalTicks];

		for (int init = 0; init < impl->totalTicks; init++)
		{
			rotationsNew[init] = rotations[init];
		}

		for (int i = 0; i < impl->totalTicks; i++)
		{
			if (selected[i])
			{
				int mini = i - smoothAmount;
				if (mini < 0)
					mini = 0;
				int maxi = i + smoothAmount;
				if (maxi > impl->totalTicks - 1)
					maxi = impl->totalTicks - 1;
				
				rotationsNew[i] = VC3(0,0,0);
				if (mini < maxi)
				{
					float totalWeight = 0;
					for (int j = mini; j < maxi; j++)
					{
						float weight = (float)smoothAmount - (float)fabs(float(j - i));
						rotationsNew[i] += rotations[j] * weight;
						totalWeight += weight;
					}
					rotationsNew[i] /= totalWeight;
				} else {
					rotationsNew[i] = rotations[i];
				}
			}
		}

		/*
		for (int fix360 = 0; fix360 < impl->totalTicks; fix360++)
		{
			while (rotationsNew[fix360] < 0)
				rotationsNew[fix360] += 360.0f;
			while (rotationsNew[fix360] >= 360.0f)
				rotationsNew[fix360] -= 360.0f;
		}
		*/

		impl->applyDataBuffers(NULL, rotationsNew, NULL);

		delete [] selected;
		delete [] rotationsNew;
		delete [] rotations;
	}

	void AniTool::smoothAim(int smoothAmount)
	{
		bool *selected = NULL;
		float *aims = NULL;

		impl->getParsedDataBuffers(&selected, NULL, NULL, &aims);

		for (int preinit = 1; preinit < impl->totalTicks; preinit++)
		{
			while (aims[preinit] > (aims[preinit - 1] + 180.0f))
			{
				aims[preinit] -= 360.0f;
			}
			while (aims[preinit] < (aims[preinit - 1] - 180.0f))
			{
				aims[preinit] += 360.0f;
			}
		}

		float *aimsNew = new float[impl->totalTicks];

		for (int init = 0; init < impl->totalTicks; init++)
		{
			aimsNew[init] = aims[init];
		}

		for (int i = 0; i < impl->totalTicks; i++)
		{
			if (selected[i])
			{
				int mini = i - smoothAmount;
				if (mini < 0)
					mini = 0;
				int maxi = i + smoothAmount;
				if (maxi > impl->totalTicks - 1)
					maxi = impl->totalTicks - 1;
				
				aimsNew[i] = 0;
				if (mini < maxi)
				{
					float totalWeight = 0;
					for (int j = mini; j < maxi; j++)
					{
						float weight = (float)smoothAmount - (float)fabs(float(j - i));
						aimsNew[i] += aims[j] * weight;
						totalWeight += weight;
					}
					aimsNew[i] /= totalWeight;
				} else {
					aimsNew[i] = aims[i];
				}
			}
		}

		/*
		for (int fix360 = 0; fix360 < impl->totalTicks; fix360++)
		{			
			Logger::getInstance()->error(int2str(aimsNew[fix360]));
		}
		*/

		/*
		for (int fix360 = 0; fix360 < impl->totalTicks; fix360++)
		{
			while (aimsNew[fix360] < 0)
				aimsNew[fix360] += 360.0f;
			while (aimsNew[fix360] >= 360.0f)
				aimsNew[fix360] -= 360.0f;
		}
		*/

		impl->applyDataBuffers(NULL, NULL, aimsNew);

		delete [] selected;
		delete [] aimsNew;
		delete [] aims;
	}

	void AniTool::deleteSelection()
	{
		impl->bufstr.erase(impl->startPosition, impl->endPosition - impl->startPosition);
		impl->calcTotalTicks();
		impl->endPosition = impl->startPosition;
		impl->keepSelectionsInsideBuffer();
	}

	void AniTool::dropMovementOnGround(GameMap *gameMap)
	{
		bool *selected = NULL;
		VC3 *positions = NULL;

		impl->getParsedDataBuffers(&selected, &positions, NULL, NULL);

		VC3 *positionsNew = new VC3[impl->totalTicks];

		for (int init = 0; init < impl->totalTicks; init++)
		{
			positionsNew[init] = positions[init];
		}

		for (int i = 0; i < impl->totalTicks; i++)
		{
			if (selected[i])
			{
				if (gameMap->isWellInScaledBoundaries(positions[i].x, positions[i].z))
				{
					positionsNew[i].y = gameMap->getScaledHeightAt(positions[i].x, positions[i].z);
				} 
			}
		}

		impl->applyDataBuffers(positionsNew, NULL, NULL);

		delete [] selected;
		delete [] positionsNew;
		delete [] positions;
	}

	void AniTool::movementSpeedFactor(float factor)
	{
		// TODO
	}

	void AniTool::loseEveryNthTick(int tickLoseInterval)
	{
		// TODO
	}

	void AniTool::removeWarps()
	{
		// TODO
	}

}

