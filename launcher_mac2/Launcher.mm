//
//  Launcher.m
//  trine
//
//  Created by Juha Hiekkamäki on 9/22/10.
//  Copyright 2010 Frozenbyte. All rights reserved.
//

#import "Launcher.h"
#include "SDL/SDL.h"
#include "LauncherParser.h"

@implementation Launcher

@synthesize window;
@synthesize languageText;
@synthesize spokenText;
@synthesize resolutionText;
@synthesize detailText;
@synthesize antialiasingText;
@synthesize anisotrophyText;
@synthesize textureText;
@synthesize physicsText;
@synthesize launchButton;
@synthesize halfButton;
@synthesize windowedButton;
@synthesize glowButton;
@synthesize distortionButton;
@synthesize soundsButton;
@synthesize videosButton;
@synthesize languageCombo;
@synthesize spokenCombo;
@synthesize resolutionCombo;
@synthesize detailCombo;
@synthesize antialiasingCombo;
@synthesize anisotrophyCombo;
@synthesize textureCombo;
@synthesize physicsCombo;

namespace {
	
	bool skipDetailUpdates = true;
	char buildDirectory[2048] = { 0 };
	
	void setStringValue(NSTextFieldCell *cell, LauncherParser *parser, const char *p)
	{
		NSString *s = [NSString stringWithUTF8String: parser->getProperty(p).c_str()];
		[cell setStringValue: s];
	}

	void setStringValue(NSButton *button, LauncherParser *parser, const char *p)
	{
		NSString *s = [NSString stringWithUTF8String: parser->getProperty(p).c_str()];
		[button setTitle: s];
	}

	void addStringValue(NSComboBox *combo, LauncherParser *parser, const char *p)
	{
		NSString *s = [NSString stringWithUTF8String: parser->getProperty(p).c_str()];
		[combo addItemWithObjectValue: s];
	}

	void setStringValue(NSComboBox *combo, LauncherParser *parser, const char *p, NSInteger index)
	{
		NSString *s = [NSString stringWithUTF8String: parser->getProperty(p).c_str()];
		[combo removeItemAtIndex: index];
		[combo insertItemWithObjectValue: s atIndex: index];
	}
	
	struct Resolution
	{
		int x;
		int y;
		bool h;
		
		Resolution()
		:	x(0)
		,	y(0)
		,	h(false)
		{
		}
	};
	
	std::vector<Resolution> resolutions;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification 
{
	// Change to actual binary dir
	char parentdir[2048] = { 0 };
	CFURLRef url0 = CFBundleCopyBundleURL(CFBundleGetMainBundle());
	CFURLRef url1 = CFURLCreateCopyAppendingPathComponent(0, url0, CFSTR("Contents"), true);
	CFURLRef url2 = CFURLCreateCopyAppendingPathComponent(0, url1, CFSTR("Resources"), true);
	if (CFURLGetFileSystemRepresentation(url2, 1, (UInt8 *)parentdir, 2048))
	{
		strcpy(buildDirectory, parentdir);
	}

	CFURLRef url3 = url2; //CFURLCreateCopyAppendingPathComponent(0, url2, CFSTR("binary"), true);
	if (CFURLGetFileSystemRepresentation(url3, 1, (UInt8 *)parentdir, 2048))
	{
		chdir(parentdir);   /* chdir to the binary app's parent */
	}

	CFRelease(url0);
	CFRelease(url1);
	CFRelease(url2);
	//CFRelease(url3);

	// Create basic confs if needed
	{
		std::string optdir = getenv("HOME") + std::string("/Library/Shadowgrounds");
		std::string condir = optdir + std::string("/config");
		std::string profdir = optdir + std::string("/profiles");
		NSString *optd = [NSString stringWithUTF8String: optdir.c_str()];
		NSString *cond = [NSString stringWithUTF8String: condir.c_str()];
		NSString *profd = [NSString stringWithUTF8String: profdir.c_str()];
		
		std::string bindir = buildDirectory;
		//bindir += "/binary";
		std::string conbin = bindir + std::string("/config");
		std::string profbin = bindir + std::string("/profiles");
		NSString *conb = [NSString stringWithUTF8String: conbin.c_str()];
		NSString *profb = [NSString stringWithUTF8String: profbin.c_str()];
		
		if(![[NSFileManager defaultManager] fileExistsAtPath:optd])
			[[NSFileManager defaultManager] createDirectoryAtPath: optd withIntermediateDirectories:TRUE attributes: nil error: nil];

		NSError *error = nil;
		if(![[NSFileManager defaultManager] fileExistsAtPath:cond])
			 [[NSFileManager defaultManager] copyItemAtPath:conb toPath: cond error:&error];

		error = nil;
		if(![[NSFileManager defaultManager] fileExistsAtPath:profd])
			[[NSFileManager defaultManager] copyItemAtPath:profb toPath: profd error:&error];
		
		//[[NSFileManager defaultManager] copyItemAtPath:srcPath toPath:dstPath error:error]
		int a = 0;
	}
	
	std::string options = getenv("HOME") + std::string("/Library/Shadowgrounds/config/options.txt");
	LauncherParser parser(options.c_str());
	
	// Resolution
	resolutions.push_back(Resolution());	
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Rect **rects = SDL_ListModes(0, SDL_OPENGL | SDL_FULLSCREEN);
	while(*rects)
	{
		SDL_Rect *r = *rects;
		int x = r->w;
		int y = r->h;
		
		Resolution re;
		re.x = x;
		re.y = y;
		resolutions.push_back(re);

		re.h = true;
		resolutions.push_back(re);
		++rects;
	}

	// Set languages
	[languageCombo removeAllItems];
	[languageCombo addItemWithObjectValue: @"English"];
	//[languageCombo addItemWithObjectValue: @"Deutsch"];
	//[languageCombo addItemWithObjectValue: @"Français"];
	//[languageCombo addItemWithObjectValue: @"Español"];
	//[languageCombo addItemWithObjectValue: @"Italiano"];
	//[languageCombo addItemWithObjectValue: @"Suomi"];
	//[languageCombo addItemWithObjectValue: @"Русский"];

	int l = parser.getPropertyInt("menu_language");
	[self changeLanguage: l];
	[languageCombo selectItemAtIndex: l];	
	[languageCombo setObjectValue:[languageCombo objectValueOfSelectedItem]];
	
	{
		int x = parser.getPropertyInt("screen_width");
		int y = parser.getPropertyInt("screen_height");
		for(int i = 0; i < resolutions.size(); ++i)
		{
			if(resolutions[i].x == x && resolutions[i].y == y)
			{
				if(parser.getPropertyInt("render_half_resolution") == 0)
					[resolutionCombo selectItemAtIndex: i];
				else
					[resolutionCombo selectItemAtIndex: i + 1];

				[resolutionCombo setObjectValue:[resolutionCombo objectValueOfSelectedItem]];
				break;
			}
		}
	}

	if(parser.getPropertyInt("windowed") == 0)
		[windowedButton setState: NSOnState];
	if(parser.getPropertyInt("render_glow") == 0)
		[glowButton setState: NSOffState];
	if(parser.getPropertyInt("render_distortion") == 0)
		[distortionButton setState: NSOffState];
	if(parser.getPropertyInt("sounds_enabled") == 0)
		[soundsButton setState: NSOffState];

	{
		int t = parser.getPropertyInt("texture_detail_level");
		if(t >= 66)
			[textureCombo selectItemAtIndex: 0];	
		else if(t >= 33)
			[textureCombo selectItemAtIndex: 1];	
		else if(t >= 0)
			[textureCombo selectItemAtIndex: 2];	
		[textureCombo setObjectValue:[textureCombo objectValueOfSelectedItem]];
	}

	{
		int a = parser.getPropertyInt("antialias_samples");
		if(a <= 1)
			[antialiasingCombo selectItemAtIndex: 0];	
		else if(a == 2)
			[antialiasingCombo selectItemAtIndex: 1];	
		else if(a >= 3)
			[antialiasingCombo selectItemAtIndex: 2];	
		[antialiasingCombo setObjectValue:[antialiasingCombo objectValueOfSelectedItem]];
	}

	{
		int a = parser.getPropertyInt("anisotrophy");
		if(a >= 8)
			[anisotrophyCombo selectItemAtIndex: 3];	
		else if(a >= 4)
			[anisotrophyCombo selectItemAtIndex: 2];	
		else if(a >= 2)
			[anisotrophyCombo selectItemAtIndex: 1];	
		else
			[anisotrophyCombo selectItemAtIndex: 0];	
		[anisotrophyCombo setObjectValue:[anisotrophyCombo objectValueOfSelectedItem]];
	}

	{
		int pf = parser.getPropertyInt("physics_fluids_enabled");
		int pmp = parser.getPropertyInt("physics_max_model_particles"); 
		int pfp = parser.getPropertyInt("physics_max_fluid_particles"); 
		
		if(pf == 1 && pfp == 500 && pmp == 1000)
			[physicsCombo selectItemAtIndex: 1];	
		else if(pf == 0 && pfp == 0 && pmp == 500)
			[physicsCombo selectItemAtIndex: 2];
		else if(pf == 0 && pfp == 0 && pmp == 100)
			[physicsCombo selectItemAtIndex: 3];
		else
			[physicsCombo selectItemAtIndex: 0];
		[physicsCombo setObjectValue:[physicsCombo objectValueOfSelectedItem]];
	}
	
	// Try to find the overall detail level
	int glowState = 0;
	if(glowButton.state == NSOnState)
		glowState = 1;
	int distortionState = 0;
	if(distortionButton.state == NSOnState)
		distortionState = 1;
	int tc = [textureCombo indexOfSelectedItem];
	int pc = [physicsCombo indexOfSelectedItem];
	int ac = [anisotrophyCombo indexOfSelectedItem];

	if(glowState && !distortionState && tc == 0 && pc == 1 && ac == 3)
		[detailCombo selectItemAtIndex: 1];
	else if(glowState && !distortionState && tc == 1 && pc == 2 && ac == 2)
		[detailCombo selectItemAtIndex: 2];
	else if(!glowState && !distortionState && tc == 2 && pc == 3 && ac == 1)
		[detailCombo selectItemAtIndex: 3];
	else
		[detailCombo selectItemAtIndex: 0];
	
	[detailCombo setObjectValue:[detailCombo objectValueOfSelectedItem]];
	skipDetailUpdates = false;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed: (NSApplication *) sender
{
	return YES;
}

- (void)comboBoxSelectionDidChange:(NSNotification *)notification
{
	if([notification object] == languageCombo)
	{
		int index = [languageCombo indexOfSelectedItem];
		[self changeLanguage: index];
	}
	else if([notification object] == detailCombo)
	{
		if(skipDetailUpdates)
			return;

		int index = [detailCombo indexOfSelectedItem];
		if(index == 1)
		{
			[glowButton setState: NSOnState];
			[distortionButton setState: NSOffState];
			[textureCombo selectItemAtIndex: 0];
			[physicsCombo selectItemAtIndex: 1];
			[anisotrophyCombo selectItemAtIndex: 3];			
		}
		else if(index == 2)
		{
			[glowButton setState: NSOnState];
			[distortionButton setState: NSOffState];
			[textureCombo selectItemAtIndex: 1];
			[physicsCombo selectItemAtIndex: 2];
			[anisotrophyCombo selectItemAtIndex: 2];			
		}
		else if(index == 3)
		{
			[glowButton setState: NSOffState];
			[distortionButton setState: NSOffState];
			[textureCombo selectItemAtIndex: 2];
			[physicsCombo selectItemAtIndex: 3];
			[anisotrophyCombo selectItemAtIndex: 1];			
		}

		// Refresh combos
		[textureCombo setObjectValue:[textureCombo objectValueOfSelectedItem]];
		[physicsCombo setObjectValue:[physicsCombo objectValueOfSelectedItem]];
		[anisotrophyCombo setObjectValue:[anisotrophyCombo objectValueOfSelectedItem]];
	}
}

- (void)changeLanguage:(int)index
{
	// Save and reset combos
	int detailIndex = [detailCombo indexOfSelectedItem];
	int aaIndex = [antialiasingCombo indexOfSelectedItem];
	int textureIndex = [textureCombo indexOfSelectedItem];
	int physicsIndex = [physicsCombo indexOfSelectedItem];
	int resolutionIndex = [resolutionCombo indexOfSelectedItem];
	[resolutionCombo removeAllItems];
	[detailCombo removeAllItems];
	[antialiasingCombo removeAllItems];
	[textureCombo removeAllItems];
	[physicsCombo removeAllItems];
	if(detailIndex <= 0)
		detailIndex = 0;
	if(aaIndex <= 0)
		aaIndex = 0;
	if(textureIndex <= 0)
		textureIndex = 0;
	if(physicsIndex <= 0)
		physicsIndex = 0;
	if(resolutionIndex <= 0)
		resolutionIndex = 0;

	LauncherParser *parser = 0;

	if(index == 0)
		parser = new LauncherParser("data/locales/en/launcher.txt");
	else if(index == 5)
		parser = new LauncherParser("data/locales/de/launcher.txt");
	else if(index == 2)
		parser = new LauncherParser("data/locales/fr/launcher.txt");
	else if(index == 3)
		parser = new LauncherParser("data/locales/es/launcher.txt");
	else if(index == 4)
		parser = new LauncherParser("data/locales/it/launcher.txt");
	else if(index == 1)
		parser = new LauncherParser("data/locales/fi/launcher.txt");
	else if(index == 6)
		parser = new LauncherParser("data/locales/RU/launcher.txt");

	setStringValue(languageText, parser, "launcher_language");
	setStringValue(resolutionText, parser, "launcher_resolution");
	setStringValue(windowedButton, parser, "launcher_fullscreen");
	setStringValue(launchButton, parser, "launcher_launch");
	setStringValue(detailText, parser, "launcher_gfx_detail_level");
	setStringValue(antialiasingText, parser, "launcher_antialiasing");
	setStringValue(anisotrophyText, parser, "launcher_anisotrophic_filtering");
	setStringValue(physicsText, parser, "launcher_physics_level");
	setStringValue(textureText, parser, "launcher_texture_detail_level");
	setStringValue(glowButton, parser, "launcher_render_glow");
	setStringValue(distortionButton, parser, "launcher_distortion_effects");
	setStringValue(soundsButton, parser, "launcher_sounds_enabled");

	for(int i = 0; i < resolutions.size(); ++i)
	{
		Resolution &r = resolutions[i];
		if(i == 0)
		{
			addStringValue(resolutionCombo, parser, "launcher_combo_custom");
			continue;
		}
		
		NSString *res = 0;
		if(r.h)
			res = [NSString stringWithFormat: @"%d x %d (%s)", r.x, r.y, parser->getProperty("launcher_combo_low").c_str()];
		else
			res = [NSString stringWithFormat: @"%d x %d", r.x, r.y];

		[resolutionCombo addItemWithObjectValue: res];
	}

	addStringValue(detailCombo, parser, "launcher_combo_custom");
	addStringValue(detailCombo, parser, "launcher_combo_high");
	addStringValue(detailCombo, parser, "launcher_combo_medium");
	addStringValue(detailCombo, parser, "launcher_combo_low");
	addStringValue(physicsCombo, parser, "launcher_combo_custom");
	addStringValue(physicsCombo, parser, "launcher_combo_high");
	addStringValue(physicsCombo, parser, "launcher_combo_medium");
	addStringValue(physicsCombo, parser, "launcher_combo_low");
	addStringValue(textureCombo, parser, "launcher_combo_high");
	addStringValue(textureCombo, parser, "launcher_combo_medium");
	addStringValue(textureCombo, parser, "launcher_combo_low");
	addStringValue(antialiasingCombo, parser, "launcher_combo_disabled");
	//addStringValue(antialiasingCombo, parser, "launcher_combo_aa_2x_samples");
	//addStringValue(antialiasingCombo, parser, "launcher_combo_aa_4x_samples");
	[antialiasingCombo addItemWithObjectValue: @"2"];
	[antialiasingCombo addItemWithObjectValue: @"4"];
	//setStringValue(resolutionCombo, parser, "launcher_combo_custom", 0);

	// Re-apply combo indices
	[detailCombo selectItemAtIndex: detailIndex];
	[antialiasingCombo selectItemAtIndex: aaIndex];
	[textureCombo selectItemAtIndex: textureIndex];
	[physicsCombo selectItemAtIndex: physicsIndex];
	[resolutionCombo selectItemAtIndex: resolutionIndex];
	// And re-apply the text
	[detailCombo setObjectValue:[detailCombo objectValueOfSelectedItem]];
	[antialiasingCombo setObjectValue:[antialiasingCombo objectValueOfSelectedItem]];
	[textureCombo setObjectValue:[textureCombo objectValueOfSelectedItem]];
	[physicsCombo setObjectValue:[physicsCombo objectValueOfSelectedItem]];
	[resolutionCombo setObjectValue:[resolutionCombo objectValueOfSelectedItem]];

	delete parser;
}

- (IBAction)launch:(id)sender
{
	// Save stuff
	std::string options = getenv("HOME") + std::string("/Library/Shadowgrounds/config/options.txt");
	LauncherParser parser(options.c_str());

	int l = [languageCombo indexOfSelectedItem];
	parser.setProperty("menu_language", l);
	parser.setProperty("subtitle_language", l);

	int r = [resolutionCombo indexOfSelectedItem];
	if(r > 0)
	{
		parser.setProperty("screen_width", resolutions[r].x);
		parser.setProperty("screen_height", resolutions[r].y);
		if(resolutions[r].h)
			parser.setProperty("render_half_resolution", 1);
		else
			parser.setProperty("render_half_resolution", 0);
	}

	if([windowedButton state] == NSOnState)
		parser.setProperty("windowed", 0);
	else 
		parser.setProperty("windowed", 1);

	if([glowButton state] == NSOnState)
		parser.setProperty("render_glow", 1);
	else 
		parser.setProperty("render_glow", 0);

	if([distortionButton state] == NSOnState)
		parser.setProperty("render_distortion", 1);
	else 
		parser.setProperty("render_distortion", 0);

	if([soundsButton state] == NSOnState)
		parser.setProperty("sounds_enabled", 1);
	else 
		parser.setProperty("sounds_enabled", 0);

	int t = [textureCombo indexOfSelectedItem];
	if(t == 0)
		parser.setProperty("texture_detail_level", 100);
	else if(t == 1)
		parser.setProperty("texture_detail_level", 50);
	else
		parser.setProperty("texture_detail_level", 0);

	int aa = [antialiasingCombo indexOfSelectedItem];
	if(aa == 0)
		parser.setProperty("antialias_samples", 1);
	else if(aa == 1)
		parser.setProperty("antialias_samples", 2);
	else if(aa == 2)
		parser.setProperty("antialias_samples", 4);

	int af = [anisotrophyCombo indexOfSelectedItem];
	if(af == 0)
		parser.setProperty("anisotrophy", 1);
	else if(af == 1)
		parser.setProperty("anisotrophy", 2);
	else if(af == 2)
		parser.setProperty("anisotrophy", 4);
	else if(af == 3)
		parser.setProperty("anisotrophy", 8);

	int p = [physicsCombo indexOfSelectedItem];
	if(p > 0)
	{
		if(p == 1)
		{
			parser.setProperty("physics_fluids_enabled", 1);
			parser.setProperty("physics_max_model_particles", 1000);
			parser.setProperty("physics_max_fluid_particles", 500);
		}
		else if(p == 2)
		{
			parser.setProperty("physics_fluids_enabled", 0);
			parser.setProperty("physics_max_model_particles", 500);
			parser.setProperty("physics_max_fluid_particles", 0);
		}
		else if(p == 3)
		{
			parser.setProperty("physics_fluids_enabled", 0);
			parser.setProperty("physics_max_model_particles", 100);
			parser.setProperty("physics_max_fluid_particles", 0);
		}
	}

	parser.save(options.c_str());

	std::string td = buildDirectory;
	/*size_t rpos = td.find("launcher.app/Contents/Resources");
	if (rpos != std::string::npos)
	{
		td.erase(rpos);
		td += "survivor.app/Contents/MacOS/survivor"; */
		td += "/shadowgrounds.app/Contents/MacOS/shadowgrounds";
		NSString *ts = [NSString stringWithUTF8String: td.c_str()];
		[[NSWorkspace sharedWorkspace] launchApplication: ts];
		[NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
	//}
}

@end
