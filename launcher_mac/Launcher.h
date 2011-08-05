//
//  Launcher.h
//  trine
//
//  Created by Juha Hiekkamäki on 9/22/10.
//  Copyright 2010 Frozenbyte. All rights reserved.
//

#import </System/Library/Frameworks/AppKit.framework/Headers/AppKit.h>
#import <Cocoa/Cocoa.h>

@interface Launcher : NSObject <NSApplicationDelegate>
{
	IBOutlet NSWindow *window;
	IBOutlet NSTextFieldCell *languageText;
	IBOutlet NSTextFieldCell *spokenText;
	IBOutlet NSTextFieldCell *resolutionText;
	IBOutlet NSTextFieldCell *detailText;
	IBOutlet NSTextFieldCell *antialiasingText;
	IBOutlet NSTextFieldCell *anisotrophyText;
	IBOutlet NSTextFieldCell *textureText;
	IBOutlet NSTextFieldCell *physicsText;
	IBOutlet NSButton *launchButton;
	IBOutlet NSButton *halfButton;
	IBOutlet NSButton *windowedButton;
	IBOutlet NSButton *glowButton;
	IBOutlet NSButton *distortionButton;
	IBOutlet NSButton *soundsButton;
	IBOutlet NSButton *videosButton;
	IBOutlet NSComboBox *languageCombo;
	IBOutlet NSComboBox *spokenCombo;
	IBOutlet NSComboBox *resolutionCombo;
	IBOutlet NSComboBox *detailCombo;
	IBOutlet NSComboBox *antialiasingCombo;
	IBOutlet NSComboBox *anisotrophyCombo;
	IBOutlet NSComboBox *textureCombo;
	IBOutlet NSComboBox *physicsCombo;
}

@property (assign) IBOutlet NSWindow *window;

@property (assign) IBOutlet NSTextFieldCell *languageText;
@property (assign) IBOutlet NSTextFieldCell *spokenText;
@property (assign) IBOutlet NSTextFieldCell *resolutionText;
@property (assign) IBOutlet NSTextFieldCell *detailText;
@property (assign) IBOutlet NSTextFieldCell *antialiasingText;
@property (assign) IBOutlet NSTextFieldCell *anisotrophyText;
@property (assign) IBOutlet NSTextFieldCell *textureText;
@property (assign) IBOutlet NSTextFieldCell *physicsText;
@property (assign) IBOutlet NSButton *launchButton;
@property (assign) IBOutlet NSButton *halfButton;
@property (assign) IBOutlet NSButton *windowedButton;
@property (assign) IBOutlet NSButton *glowButton;
@property (assign) IBOutlet NSButton *distortionButton;
@property (assign) IBOutlet NSButton *soundsButton;
@property (assign) IBOutlet NSButton *videosButton;
@property (assign) IBOutlet NSComboBox *languageCombo;
@property (assign) IBOutlet NSComboBox *spokenCombo;
@property (assign) IBOutlet NSComboBox *resolutionCombo;
@property (assign) IBOutlet NSComboBox *detailCombo;
@property (assign) IBOutlet NSComboBox *antialiasingCombo;
@property (assign) IBOutlet NSComboBox *anisotrophyCombo;
@property (assign) IBOutlet NSComboBox *textureCombo;
@property (assign) IBOutlet NSComboBox *physicsCombo;

- (void)changeLanguage:(int)index;
- (IBAction)launch:(id)sender;

@end
