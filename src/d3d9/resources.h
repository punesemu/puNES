/*
 * resources.h
 *
 *  Created on: 23/lug/2011
 *      Author: fhorse
 */

#ifndef RESOURCES_H_
#define RESOURCES_H_

#define IDI_MYICON                  101
#define IDB_ABOUT                   102

#define IDD_ABOUT                   150
#define IDD_ABOUT_PORTABLE          151

#define IDD_INPUT_CONTROLLERS       160
#define IDD_STD_CTRL                161

#define IDT_TIMER1                  101
#define IDT_TIMER2                  102

#define IDR_MAINMENU                200
#define IDM_FILE_OPEN               201
#define IDM_FILE_EXIT               202

#define IDM_FILE_RECENT             210
#define IDM_FILE_RECENT_0           211
#define IDM_FILE_RECENT_1           212
#define IDM_FILE_RECENT_2           213
#define IDM_FILE_RECENT_3           214
#define IDM_FILE_RECENT_4           215
#define IDM_FILE_RECENT_5           216
#define IDM_FILE_RECENT_6           217
#define IDM_FILE_RECENT_7           218
#define IDM_FILE_RECENT_8           219
#define IDM_FILE_RECENT_9           220
#define IDM_FILE_RECENT_10          221
#define IDM_FILE_RECENT_11          222
#define IDM_FILE_RECENT_12          223
#define IDM_FILE_RECENT_13          224
#define IDM_FILE_RECENT_14          225

#define IDM_SET_MODE_PAL            300
#define IDM_SET_MODE_NTSC           301
#define IDM_SET_MODE_DENDY          302
#define IDM_SET_MODE_AUTO           303

#define IDM_SET_FPS_DEFAULT         320
#define IDM_SET_FPS_60              321
#define IDM_SET_FPS_59              322
#define IDM_SET_FPS_58              323
#define IDM_SET_FPS_57              324
#define IDM_SET_FPS_56              325
#define IDM_SET_FPS_55              326
#define IDM_SET_FPS_54              327
#define IDM_SET_FPS_53              328
#define IDM_SET_FPS_52              329
#define IDM_SET_FPS_51              330
#define IDM_SET_FPS_50              331
#define IDM_SET_FPS_49              332
#define IDM_SET_FPS_48              333
#define IDM_SET_FPS_47              334
#define IDM_SET_FPS_46              335
#define IDM_SET_FPS_45              336
#define IDM_SET_FPS_44              337

#define IDM_SET_FSK_DEFAULT         340
#define IDM_SET_FSK_1               341
#define IDM_SET_FSK_2               342
#define IDM_SET_FSK_3               343
#define IDM_SET_FSK_4               344
#define IDM_SET_FSK_5               345
#define IDM_SET_FSK_6               346
#define IDM_SET_FSK_7               347
#define IDM_SET_FSK_8               348
#define IDM_SET_FSK_9               349

#define IDM_SET_SIZE_1X             360
#define IDM_SET_SIZE_2X             361
#define IDM_SET_SIZE_3X             362
#define IDM_SET_SIZE_4X             363

#define IDM_SET_OSCAN_DEF           370
#define IDM_SET_OSCAN_ON            371
#define IDM_SET_OSCAN_OFF           372
#define IDM_SET_OSCAN_DEFAULT_ON    373
#define IDM_SET_OSCAN_DEFAULT_OFF   374

#define IDM_SET_FILTER_NO_FILTER    380
#define IDM_SET_FILTER_BILINEAR     381
#define IDM_SET_FILTER_POSPHOR      382
#define IDM_SET_FILTER_SCANLINE     383
#define IDM_SET_FILTER_DBL          384
#define IDM_SET_FILTER_CRTCURVE     385
#define IDM_SET_FILTER_CRTNOCURVE   386
#define IDM_SET_FILTER_SCALE2X      387
#define IDM_SET_FILTER_SCALE3X      388
#define IDM_SET_FILTER_SCALE4X      389
#define IDM_SET_FILTER_HQ2X         390
#define IDM_SET_FILTER_HQ3X         391
#define IDM_SET_FILTER_HQ4X         392
#define IDM_SET_FILTER_RGBNTSCCOM   393
#define IDM_SET_FILTER_RGBNTSCSVD   394
#define IDM_SET_FILTER_RGBNTSCRGB   395

#define IDM_SET_PALETTE_PAL         410
#define IDM_SET_PALETTE_NTSC        411
#define IDM_SET_PALETTE_SONY        412
#define IDM_SET_PALETTE_MONO        413
#define IDM_SET_PALETTE_GREEN       414

#define IDM_SET_RENDERING_SOFTWARE  420
#define IDM_SET_RENDERING_HLSL      421
#define IDM_SET_EFFECT_CUBE         422
#define IDM_SET_VSYNC_ON            423
#define IDM_SET_VSYNC_OFF           424
#define IDM_SET_FULLSCREEN          425
#define IDM_SET_STRETCHFLSCR        426

#define IDM_SET_SAVENOW             430
#define IDM_SET_SAVEONEXIT          431

#define IDM_NES_HARD                500
#define IDM_NES_SOFT                501
#define IDM_NES_FDS_DISK_SIDE0      502
#define IDM_NES_FDS_DISK_SIDE1      503
#define IDM_NES_FDS_DISK_SIDE2      504
#define IDM_NES_FDS_DISK_SIDE3      505
#define IDM_NES_FDS_DISK_SIDE4      506
#define IDM_NES_FDS_DISK_SIDE5      507
#define IDM_NES_FDS_DISK_SIDE6      508
#define IDM_NES_FDS_DISK_SIDE7      509
#define IDM_NES_FDS_DISK_SWITCH     510
#define IDM_NES_FDS_EJECT           511

#define IDM_HELP_ABOUT              600

#define IDM_SET_SAVE_SAVE           700
#define IDM_SET_SAVE_LOAD           701
#define IDM_SET_SAVE_INC            702
#define	IDM_SET_SAVE_DEC            703
#define	IDM_SET_SAVE_0              704
#define	IDM_SET_SAVE_1              705
#define IDM_SET_SAVE_2              706
#define IDM_SET_SAVE_3              707
#define IDM_SET_SAVE_4              708
#define IDM_SET_SAVE_5              709

#define IDM_SET_INPUT_CONFIG        800

#define IDM_SET_SAMPLERATE_44100    900
#define IDM_SET_SAMPLERATE_22050    901
#define IDM_SET_SAMPLERATE_11025    902

#define IDM_SET_CHANNELS_MONO       910
#define IDM_SET_CHANNELS_STEREO     911

#define IDM_SET_STEREO_DELAY_5      920
#define IDM_SET_STEREO_DELAY_10     921
#define IDM_SET_STEREO_DELAY_15     922
#define IDM_SET_STEREO_DELAY_20     923
#define IDM_SET_STEREO_DELAY_25     924
#define IDM_SET_STEREO_DELAY_30     925
#define IDM_SET_STEREO_DELAY_35     926
#define IDM_SET_STEREO_DELAY_40     927
#define IDM_SET_STEREO_DELAY_45     928
#define IDM_SET_STEREO_DELAY_50     929
#define IDM_SET_STEREO_DELAY_55     930
#define IDM_SET_STEREO_DELAY_60     931
#define IDM_SET_STEREO_DELAY_65     932
#define IDM_SET_STEREO_DELAY_70     933
#define IDM_SET_STEREO_DELAY_75     934
#define IDM_SET_STEREO_DELAY_80     935
#define IDM_SET_STEREO_DELAY_85     936
#define IDM_SET_STEREO_DELAY_90     937
#define IDM_SET_STEREO_DELAY_95     938
#define IDM_SET_STEREO_DELAY_100    939

#define IDM_SET_AUDIO_QUALITY_LOW   950
#define IDM_SET_AUDIO_QUALITY_HIGH  951

#define IDM_SET_AUDIO_SWAP_DUTY     960
#define IDM_SET_AUDIO_ENABLE        961

#define IDM_SET_GAMEGENIE           970

#define ID_SAVESLOT_CB              100
#define ID_SAVESLOT_BS              101
#define ID_SAVESLOT_BL              102

#define IDC_INPUT_CTRL_PORT1_C      100
#define IDC_INPUT_CTRL_PORT1_B      101
#define IDC_INPUT_CTRL_PORT2_C      102
#define IDC_INPUT_CTRL_PORT2_B      103

#define IDC_STD_CTRL_KEY_A          200
#define IDC_STD_CTRL_KEY_B          201
#define IDC_STD_CTRL_KEY_SELECT     202
#define IDC_STD_CTRL_KEY_START      203
#define IDC_STD_CTRL_KEY_UP         204
#define IDC_STD_CTRL_KEY_DOWN       205
#define IDC_STD_CTRL_KEY_LEFT       206
#define IDC_STD_CTRL_KEY_RIGHT      207
#define IDC_STD_CTRL_KEY_TURBOA     208
#define IDC_STD_CTRL_KEY_TURBOB     209
#define IDC_STD_CTRL_KEY_ERASE      210
#define IDC_STD_CTRL_JOY_A          211
#define IDC_STD_CTRL_JOY_B          212
#define IDC_STD_CTRL_JOY_SELECT     213
#define IDC_STD_CTRL_JOY_START      214
#define IDC_STD_CTRL_JOY_UP         215
#define IDC_STD_CTRL_JOY_DOWN       216
#define IDC_STD_CTRL_JOY_LEFT       217
#define IDC_STD_CTRL_JOY_RIGHT      218
#define IDC_STD_CTRL_JOY_TURBOA     219
#define IDC_STD_CTRL_JOY_TURBOB     220
#define IDC_STD_CTRL_JOY_ID         221
#define IDC_STD_CTRL_JOY_ERASE      222

#endif /* RESOURCES_H_ */
