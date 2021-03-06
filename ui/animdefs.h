#ifndef ANIMDEFS_H
#define ANIMDEFS_H

#define MAX_ANIMATIONS              512

#define ANIM_TYPES_AMOUNT           8

#define ANIM_NONE                   0
#define ANIM_STAND                  1
#define ANIM_WALK                   2
#define ANIM_RUN                    3

#define ANIM_DIE_BACK               4
#define ANIM_DIE_FRONT              5
#define ANIM_DIE_IMPACT_BACK        6
#define ANIM_DIE_IMPACT_FRONT       7

#define ANIM_SHOOT_LEFT_ARM         8
#define ANIM_SHOOT_RIGHT_ARM        9

#define ANIM_WALK_DAMAGED           10
#define ANIM_RUN_DAMAGED            11

#define ANIM_STAGGER_BACKWARD       12
#define ANIM_STAGGER_FORWARD        13

#define ANIM_AIM_LEFT_ARM           14
#define ANIM_AIM_RIGHT_ARM          15

// NOTICE: the idle anim numbers must follow each other
#define ANIM_IDLE1                  16
#define ANIM_IDLE2                  17
#define ANIM_IDLE3                  18
#define ANIM_IDLE4                  19
#define ANIM_IDLE5                  20
#define ANIM_IDLE6                  21
#define ANIM_IDLE7                  22
#define ANIM_IDLE8                  23

#define ANIM_SHOOT_HEAVY_LEFT       24
#define ANIM_SHOOT_HEAVY_RIGHT      25
#define ANIM_SHOOT_MEDIUM_LEFT      26
#define ANIM_SHOOT_MEDIUM_RIGHT     27
#define ANIM_SHOOT_RAPID_LEFT       28
#define ANIM_SHOOT_RAPID_RIGHT      29
#define ANIM_SHOOT_ULTRARAPID_LEFT  30
#define ANIM_SHOOT_ULTRARAPID_RIGHT 31

#define ANIM_SHOOT                  32

#define ANIM_CROUCH                 33
#define ANIM_SHOOT_CROUCH           34

#define ANIM_PRONE                  35
#define ANIM_SHOOT_PRONE            36
#define ANIM_CRAWL                  37
#define ANIM_GO_PRONE               38
#define ANIM_RISE_PRONE             39

#define ANIM_DRIVE                  40

#define ANIM_DIE                    41

#define ANIM_SPRINT                 42

#define ANIM_STAGGER_LEFT           43
#define ANIM_STAGGER_RIGHT          44

#define ANIM_AIM_PRONE              45

#define ANIM_GET_UP_BACKDOWN        46
#define ANIM_GET_UP_FACEDOWN        47

#define ANIM_FLIP_SIDE_LEFT         48
#define ANIM_FLIP_SIDE_RIGHT        49

#define ANIM_AIM_BOTH_ARMS          50

#define ANIM_TORSOTWIST_LEFT        51
#define ANIM_TORSOTWIST_RIGHT       52

#define ANIM_AIM                    53

#define ANIM_DIE_PRONE              54

#define ANIM_STRAFE_LEFT            55
#define ANIM_STRAFE_RIGHT           56
#define ANIM_RUN_BACKWARD           57

#define ANIM_GIVE                   58

#define ANIM_AIM_TYPE0              59
#define ANIM_AIM_TYPE1              60
#define ANIM_AIM_TYPE2              61
#define ANIM_AIM_TYPE3              62
#define ANIM_AIM_TYPE4              63
#define ANIM_AIM_TYPE5              64
#define ANIM_AIM_TYPE6              65
#define ANIM_AIM_TYPE7              66

#define ANIM_SHOOT_TYPE0            67
#define ANIM_SHOOT_TYPE1            68
#define ANIM_SHOOT_TYPE2            69
#define ANIM_SHOOT_TYPE3            70
#define ANIM_SHOOT_TYPE4            71
#define ANIM_SHOOT_TYPE5            72
#define ANIM_SHOOT_TYPE6            73
#define ANIM_SHOOT_TYPE7            74

#define ANIM_STAND_TYPE0            75
#define ANIM_STAND_TYPE1            76
#define ANIM_STAND_TYPE2            77
#define ANIM_STAND_TYPE3            78
#define ANIM_STAND_TYPE4            79
#define ANIM_STAND_TYPE5            80
#define ANIM_STAND_TYPE6            81
#define ANIM_STAND_TYPE7            82

#define ANIM_WALK_TYPE0             83
#define ANIM_WALK_TYPE1             84
#define ANIM_WALK_TYPE2             85
#define ANIM_WALK_TYPE3             86
#define ANIM_WALK_TYPE4             87
#define ANIM_WALK_TYPE5             88
#define ANIM_WALK_TYPE6             89
#define ANIM_WALK_TYPE7             90

#define ANIM_RUN_TYPE0              91
#define ANIM_RUN_TYPE1              92
#define ANIM_RUN_TYPE2              93
#define ANIM_RUN_TYPE3              94
#define ANIM_RUN_TYPE4              95
#define ANIM_RUN_TYPE5              96
#define ANIM_RUN_TYPE6              97
#define ANIM_RUN_TYPE7              98

#define ANIM_SPRINT_TYPE0           99
#define ANIM_SPRINT_TYPE1           100
#define ANIM_SPRINT_TYPE2           101
#define ANIM_SPRINT_TYPE3           102
#define ANIM_SPRINT_TYPE4           103
#define ANIM_SPRINT_TYPE5           104
#define ANIM_SPRINT_TYPE6           105
#define ANIM_SPRINT_TYPE7           106

#define ANIM_RUN_BACKWARD_TYPE0     107
#define ANIM_RUN_BACKWARD_TYPE1     108
#define ANIM_RUN_BACKWARD_TYPE2     109
#define ANIM_RUN_BACKWARD_TYPE3     110
#define ANIM_RUN_BACKWARD_TYPE4     111
#define ANIM_RUN_BACKWARD_TYPE5     112
#define ANIM_RUN_BACKWARD_TYPE6     113
#define ANIM_RUN_BACKWARD_TYPE7     114

#define ANIM_JUMP                   115

#define ANIM_SPECIAL1               116
#define ANIM_SPECIAL2               117
#define ANIM_SPECIAL3               118
#define ANIM_SPECIAL4               119
#define ANIM_SPECIAL5               120
#define ANIM_SPECIAL6               121
#define ANIM_SPECIAL7               122
#define ANIM_SPECIAL8               123

#define ANIM_SPECIAL9               124
#define ANIM_SPECIAL10              125
#define ANIM_SPECIAL11              126
#define ANIM_SPECIAL12              127
#define ANIM_SPECIAL13              128
#define ANIM_SPECIAL14              129
#define ANIM_SPECIAL15              130
#define ANIM_SPECIAL16              131

#define ANIM_SPECIAL17              132
#define ANIM_SPECIAL18              133
#define ANIM_SPECIAL19              134
#define ANIM_SPECIAL20              135
#define ANIM_SPECIAL21              136
#define ANIM_SPECIAL22              137
#define ANIM_SPECIAL23              138
#define ANIM_SPECIAL24              139

#define ANIM_SPECIAL25              140
#define ANIM_SPECIAL26              141
#define ANIM_SPECIAL27              142
#define ANIM_SPECIAL28              143
#define ANIM_SPECIAL29              144
#define ANIM_SPECIAL30              145
#define ANIM_SPECIAL31              146
#define ANIM_SPECIAL32              147

#define ANIM_SPECIAL33              148
#define ANIM_SPECIAL34              149
#define ANIM_SPECIAL35              150
#define ANIM_SPECIAL36              151
#define ANIM_SPECIAL37              152
#define ANIM_SPECIAL38              153
#define ANIM_SPECIAL39              154
#define ANIM_SPECIAL40              155

#define ANIM_SPECIAL41              156
#define ANIM_SPECIAL42              157
#define ANIM_SPECIAL43              158
#define ANIM_SPECIAL44              159
#define ANIM_SPECIAL45              160

#define ANIM_STRAFE_LEFT_TYPE0      161
#define ANIM_STRAFE_LEFT_TYPE1      162
#define ANIM_STRAFE_LEFT_TYPE2      163
#define ANIM_STRAFE_LEFT_TYPE3      164
#define ANIM_STRAFE_LEFT_TYPE4      165
#define ANIM_STRAFE_LEFT_TYPE5      166
#define ANIM_STRAFE_LEFT_TYPE6      167
#define ANIM_STRAFE_LEFT_TYPE7      168

#define ANIM_STRAFE_RIGHT_TYPE0     169
#define ANIM_STRAFE_RIGHT_TYPE1     170
#define ANIM_STRAFE_RIGHT_TYPE2     171
#define ANIM_STRAFE_RIGHT_TYPE3     172
#define ANIM_STRAFE_RIGHT_TYPE4     173
#define ANIM_STRAFE_RIGHT_TYPE5     174
#define ANIM_STRAFE_RIGHT_TYPE6     175
#define ANIM_STRAFE_RIGHT_TYPE7     176

#define ANIM_JUMP_BACKWARD          177
#define ANIM_JUMP_LEFT              178
#define ANIM_JUMP_RIGHT             179

#define ANIM_PUSH_BUTTON            180

#define ANIM_ELECTRIFIED            181
#define ANIM_STUNNED                182

#define ANIM_TURN_LEFT              183
#define ANIM_TURN_RIGHT             184

#define ANIM_TURN_LEFT_TYPE0        185
#define ANIM_TURN_LEFT_TYPE1        186
#define ANIM_TURN_LEFT_TYPE2        187
#define ANIM_TURN_LEFT_TYPE3        188
#define ANIM_TURN_LEFT_TYPE4        189
#define ANIM_TURN_LEFT_TYPE5        190
#define ANIM_TURN_LEFT_TYPE6        191
#define ANIM_TURN_LEFT_TYPE7        192

#define ANIM_TURN_RIGHT_TYPE0       193
#define ANIM_TURN_RIGHT_TYPE1       194
#define ANIM_TURN_RIGHT_TYPE2       195
#define ANIM_TURN_RIGHT_TYPE3       196
#define ANIM_TURN_RIGHT_TYPE4       197
#define ANIM_TURN_RIGHT_TYPE5       198
#define ANIM_TURN_RIGHT_TYPE6       199
#define ANIM_TURN_RIGHT_TYPE7       200

#define ANIM_DIE_POISON             201

#define ANIM_DIE_BACK2              202
#define ANIM_DIE_FRONT2             203
#define ANIM_DIE_IMPACT_BACK2       204
#define ANIM_DIE_IMPACT_FRONT2      205

#define ANIM_SHOOT_TYPE0_STANDING   206
#define ANIM_SHOOT_TYPE1_STANDING   207
#define ANIM_SHOOT_TYPE2_STANDING   208
#define ANIM_SHOOT_TYPE3_STANDING   209
#define ANIM_SHOOT_TYPE4_STANDING   210
#define ANIM_SHOOT_TYPE5_STANDING   211
#define ANIM_SHOOT_TYPE6_STANDING   212
#define ANIM_SHOOT_TYPE7_STANDING   213

#define ANIM_RELOAD_CLIP_TYPE0      214
#define ANIM_RELOAD_CLIP_TYPE1      215
#define ANIM_RELOAD_CLIP_TYPE2      216
#define ANIM_RELOAD_CLIP_TYPE3      217
#define ANIM_RELOAD_CLIP_TYPE4      218
#define ANIM_RELOAD_CLIP_TYPE5      219
#define ANIM_RELOAD_CLIP_TYPE6      220
#define ANIM_RELOAD_CLIP_TYPE7      221

#define ANIM_DIE_HEAT               222
#define ANIM_DIE_HEAT2              223

// last animation + 1
#define ANIM_AMOUNT                 224

#endif
