/*
 * resources.h
 *
 *  Created on: 23/lug/2011
 *      Author: fhorse
 */

#ifndef RESOURCES_H_
#define RESOURCES_H_

#define IDI_MYICON                 101
#define IDB_ABOUT                  102

#define IDD_ABOUT                  150

#define IDD_INPUT_CONTROLLERS      160
#define IDD_STD_CTRL               161

#define IDT_TIMER1                 101
#define IDT_TIMER2                 102

#ifndef SDL
#define IDR_MAINMENU               200
#define IDM_FILE_OPEN              201
#define IDM_FILE_EXIT              202

#define IDM_SET_MODE_PAL           300
#define IDM_SET_MODE_NTSC          301
#define IDM_SET_MODE_DENDY         302
#define IDM_SET_MODE_AUTO          303

#define IDM_SET_FPS_DEFAULT        320
#define IDM_SET_FPS_58             321
#define IDM_SET_FPS_57             322
#define IDM_SET_FPS_56             323
#define IDM_SET_FPS_55             324
#define IDM_SET_FPS_54             325
#define IDM_SET_FPS_53             326
#define IDM_SET_FPS_52             327
#define IDM_SET_FPS_51             328
#define IDM_SET_FPS_50             329
#define IDM_SET_FPS_49             330
#define IDM_SET_FPS_48             331
#define IDM_SET_FPS_47             332
#define IDM_SET_FPS_46             333
#define IDM_SET_FPS_45             334
#define IDM_SET_FPS_44             335

#define IDM_SET_FSK_DEFAULT        340
#define IDM_SET_FSK_1              341
#define IDM_SET_FSK_2              342
#define IDM_SET_FSK_3              343
#define IDM_SET_FSK_4              344
#define IDM_SET_FSK_5              345
#define IDM_SET_FSK_6              346
#define IDM_SET_FSK_7              347
#define IDM_SET_FSK_8              348
#define IDM_SET_FSK_9              349

#define IDM_SET_SIZE_1X            360
#define IDM_SET_SIZE_2X            361
#define IDM_SET_SIZE_3X            362
#define IDM_SET_SIZE_4X            363

#define IDM_SET_OSCAN_DEF          370
#define IDM_SET_OSCAN_ON           371
#define IDM_SET_OSCAN_OFF          372
#define IDM_SET_OSCAN_DEFAULT_ON   373
#define IDM_SET_OSCAN_DEFAULT_OFF  374

#define IDM_SET_FILTER_NOFILTER    380
#define IDM_SET_FILTER_BILINEAR    381
#define IDM_SET_FILTER_POSPHOR     382
#define IDM_SET_FILTER_SCANLINE    383
#define IDM_SET_FILTER_CRT         384
#define IDM_SET_FILTER_SCALE2X     385
#define IDM_SET_FILTER_SCALE3X     386
#define IDM_SET_FILTER_SCALE4X     387
#define IDM_SET_FILTER_HQ2X        388
#define IDM_SET_FILTER_HQ3X        389
#define IDM_SET_FILTER_HQ4X        390
#define IDM_SET_FILTER_RGBNTSCCOM  391
#define IDM_SET_FILTER_RGBNTSCSVD  392
#define IDM_SET_FILTER_RGBNTSCRGB  393

#define IDM_SET_PALETTE_PAL        395
#define IDM_SET_PALETTE_NTSC       396
#define IDM_SET_PALETTE_SONY       397
#define IDM_SET_PALETTE_MONO       398
#define IDM_SET_PALETTE_GREEN      399

#ifdef OPENGL
#define IDM_SET_RENDERING_SOFTWARE 400
#define IDM_SET_RENDERING_OPENGL   401
#define IDM_SET_EFFECT_CUBE        402
#define IDM_SET_VSYNC_ON           403
#define IDM_SET_VSYNC_OFF          404
#define IDM_SET_FULLSCREEN         405
#define IDM_SET_STRETCHFLSCR       406
#endif

#define IDM_SET_SAVENOW            420
#define IDM_SET_SAVEONEXIT         421

#define IDM_NES_HARD               500
#define IDM_NES_SOFT               501
#define IDM_NES_FDS_DISK_SIDE0     502
#define IDM_NES_FDS_DISK_SIDE1     503
#define IDM_NES_FDS_DISK_SIDE2     504
#define IDM_NES_FDS_DISK_SIDE3     505
#define IDM_NES_FDS_DISK_SIDE4     506
#define IDM_NES_FDS_DISK_SIDE5     507
#define IDM_NES_FDS_DISK_SIDE6     508
#define IDM_NES_FDS_DISK_SIDE7     509
#define IDM_NES_FDS_DISK_SWITCH    510
#define IDM_NES_FDS_EJECT          511

#define IDM_HELP_ABOUT             600

#define IDM_SET_SAVE_SAVE          700
#define IDM_SET_SAVE_LOAD          701
#define IDM_SET_SAVE_INC           702
#define	IDM_SET_SAVE_DEC           703
#define	IDM_SET_SAVE_0             704
#define	IDM_SET_SAVE_1             705
#define IDM_SET_SAVE_2             706
#define IDM_SET_SAVE_3             707
#define IDM_SET_SAVE_4             708
#define IDM_SET_SAVE_5             709

#define IDM_SET_INPUT_CONFIG       800

#define IDM_SET_SAMPLERATE_44100   900
#define IDM_SET_SAMPLERATE_22050   901
#define IDM_SET_SAMPLERATE_11025   902

#define IDM_SET_CHANNELS_MONO      910
#define IDM_SET_CHANNELS_STEREO    911

#define IDM_SET_AUDIO_ENABLE       920

#define IDM_SET_GAMEGENIE          930

#define ID_SAVESLOT_CB             100
#define ID_SAVESLOT_BS             101
#define ID_SAVESLOT_BL             102
#endif

#define IDC_INPUT_CTRL_PORT1_C     100
#define IDC_INPUT_CTRL_PORT1_B     101
#define IDC_INPUT_CTRL_PORT2_C     102
#define IDC_INPUT_CTRL_PORT2_B     103

#define IDC_STD_CTRL_KEY_A         200
#define IDC_STD_CTRL_KEY_B         201
#define IDC_STD_CTRL_KEY_SELECT    202
#define IDC_STD_CTRL_KEY_START     203
#define IDC_STD_CTRL_KEY_UP        204
#define IDC_STD_CTRL_KEY_DOWN      205
#define IDC_STD_CTRL_KEY_LEFT      206
#define IDC_STD_CTRL_KEY_RIGHT     207
#define IDC_STD_CTRL_KEY_TURBOA    208
#define IDC_STD_CTRL_KEY_TURBOB    209
#define IDC_STD_CTRL_JOY_A         210
#define IDC_STD_CTRL_JOY_B         211
#define IDC_STD_CTRL_JOY_SELECT    212
#define IDC_STD_CTRL_JOY_START     213
#define IDC_STD_CTRL_JOY_UP        214
#define IDC_STD_CTRL_JOY_DOWN      215
#define IDC_STD_CTRL_JOY_LEFT      216
#define IDC_STD_CTRL_JOY_RIGHT     217
#define IDC_STD_CTRL_JOY_TURBOA    218
#define IDC_STD_CTRL_JOY_TURBOB    219
#define IDC_STD_CTRL_JOY_ID        220

#endif /* RESOURCES_H_ */
