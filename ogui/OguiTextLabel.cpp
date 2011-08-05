#include "precompiled.h"

#include <stdlib.h>
#include <string.h>
#include "OguiTextLabel.h"
#include "OguiStormDriver.h"
#include "../util/UnicodeConverter.h"
#include "../util/Debug_MemoryManager.h"


OguiTextLabel::OguiTextLabel(OguiButton *but)
{
  this->implButton = but;
}

OguiTextLabel::~OguiTextLabel()
{
  if (implButton == NULL)
  {
    abort(); // we've got a bug
  }
  delete implButton;
  implButton = NULL;
}

void OguiTextLabel::Move(int x, int y)
{
  implButton->Move(x, y);
}

void OguiTextLabel::Resize(int x, int y)
{
  implButton->Resize(x, y);
}

void OguiTextLabel::SetTextHAlign(OguiButton::TEXT_H_ALIGN hAlign)
{
  implButton->SetTextHAlign(hAlign);
}

void OguiTextLabel::SetTextVAlign(OguiButton::TEXT_V_ALIGN vAlign)
{
  implButton->SetTextVAlign(vAlign);
}

void OguiTextLabel::SetFont(IOguiFont *font)
{
  implButton->SetFont(font);
}

void OguiTextLabel::SetText(const char *text)
{
	IOguiFont *font = implButton->GetFont();

	// text area behaviour
	if(implButton->IsLineBreaks() && font)
	{
		OguiStormFont *implFont = (OguiStormFont *) (font);
		IStorm3D_Font *stormFont = implFont->fnt;
		
		if(stormFont->isUnicode())
		{
			std::wstring unicode;
			util::convertToWide(text, unicode);

			{
				int lastbr = 0;
				int lastspace = -1;
				int textlen = unicode.size();
				unicode += wchar_t('\0');

				for (int i = 0; i < textlen + 1; i++)
				{
					if (unicode[i] == ' ' || unicode[i] == '\n' || unicode[i] == '\0')
					{
						int sizex = implButton->GetSizeX();
						wchar_t tmpchar = unicode[i];
						unicode[i] = '\0';
						if (implButton->GetFont() && implButton->GetFont()->getStringWidth(&unicode[lastbr]) > sizex)
						{
							if (lastspace != -1)
							{
								unicode[lastspace] = '\n';
								lastbr = lastspace + 1;
								lastspace = i;
							} else {
								if (tmpchar == ' ') 
								{
									tmpchar = '\n';
									lastbr = i + 1;
									lastspace = -1;
								} else {
									lastbr = i + 1;
								}
							}
						} else {
							if (tmpchar == ' ') lastspace = i;
							if (tmpchar == '\n') lastbr = i + 1;
						}
						unicode[i] = tmpchar;
					}
				}
			}

			std::string result;
			util::convertToMultiByte(unicode, result);

			implButton->SetText(result.c_str());
		}
		else
		{
			// wrap the text into lines, attempt to keep it in width boundary
			int lastbr = 0;
			int lastspace = -1;
			int textlen = strlen(text);
			char *brtext = new char[textlen + 1];
			strcpy(brtext, text);
			for (int i = 0; i < textlen + 1; i++)
			{
				if (brtext[i] == ' ' || brtext[i] == '\n' || brtext[i] == '\0')
				{
					int sizex = implButton->GetSizeX();
					char tmpchar = brtext[i];
					brtext[i] = '\0';
					if ( implButton->GetFont() && implButton->GetFont()->getStringWidth(&brtext[lastbr]) > sizex)
					{
						if (lastspace != -1)
						{
							brtext[lastspace] = '\n';
							lastbr = lastspace + 1;
							lastspace = i;
						} else {
							if (tmpchar == ' ') 
							{
								tmpchar = '\n';
								lastbr = i + 1;
								lastspace = -1;
							} else {
								lastbr = i + 1;
							}
						}
					} else {
						if (tmpchar == ' ') lastspace = i;
						if (tmpchar == '\n') lastbr = i + 1;
					}
					brtext[i] = tmpchar;
				}
			}

			implButton->SetText(brtext);
			delete [] brtext;
		}

	} else {
		// text label behaviour
		implButton->SetText(text);
	}
}

void OguiTextLabel::SetLinebreaks(bool lineBreaks)
{
	implButton->SetLineBreaks(lineBreaks);
}

// added by Pete
void OguiTextLabel::SetTransparency( int transp_pros )
{
	implButton->SetTransparency( transp_pros );
}

OguiButton *OguiTextLabel::GetButton()
{
	return implButton;
}
