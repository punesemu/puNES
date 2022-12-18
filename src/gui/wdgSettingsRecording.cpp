/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <QtGui/QStandardItemModel>
#include "wdgSettingsRecording.hpp"
#include "mainWindow.hpp"
#include "recording.h"
#include "conf.h"
#include "settings.h"

#if defined (WITH_FFMPEG)
static const char *format_description[REC_FORMAT_TOTAL] = {
//: Do not translate file extensions contained between parentheses [example: (*.mp3)]
/* REC_FORMAT_VIDEO_MPG_MPEG1 */ QT_TRANSLATE_NOOP("wdgSettingsRecording", "MPEG 1 Video (*.mpg *.mpeg)"),
//: Do not translate file extensions contained between parentheses [example: (*.mp3)]
/* REC_FORMAT_VIDEO_MPG_MPEG2 */ QT_TRANSLATE_NOOP("wdgSettingsRecording", "MPEG 2 Video (*.mpg *.mpeg)"),
//: Do not translate file extensions contained between parentheses [example: (*.mp3)]
/* REC_FORMAT_VIDEO_MP4_MPEG4 */ QT_TRANSLATE_NOOP("wdgSettingsRecording", "MPEG 4 Video (*.mp4)"),
//: Do not translate file extensions contained between parentheses [example: (*.mp3)]
/* REC_FORMAT_VIDEO_MP4_H264  */ QT_TRANSLATE_NOOP("wdgSettingsRecording", "MPEG H264 Video (*.mp4)"),
//: Do not translate file extensions contained between parentheses [example: (*.mp3)]
/* REC_FORMAT_VIDEO_MKV_HEVC  */ QT_TRANSLATE_NOOP("wdgSettingsRecording", "High Efficiency Video Codec (*.mkv)"),
//: Do not translate file extensions contained between parentheses [example: (*.mp3)]
/* REC_FORMAT_VIDEO_WEB_WEBM  */ QT_TRANSLATE_NOOP("wdgSettingsRecording", "WebM Video (*.webm)"),
//: Do not translate file extensions contained between parentheses [example: (*.mp3)]
/* REC_FORMAT_VIDEO_AVI_WMV   */ QT_TRANSLATE_NOOP("wdgSettingsRecording", "Windows Media Video (*.wmv)"),
//: Do not translate file extensions contained between parentheses [example: (*.mp3)]
/* REC_FORMAT_VIDEO_AVI_FFV   */ QT_TRANSLATE_NOOP("wdgSettingsRecording", "AVI FF Video (*.avi)"),
//: Do not translate file extensions contained between parentheses [example: (*.mp3)]
/* REC_FORMAT_VIDEO_AVI_RAW   */ QT_TRANSLATE_NOOP("wdgSettingsRecording", "AVI Video (*.avi)"),
//: Do not translate file extensions contained between parentheses [example: (*.mp3)]
/* REC_FORMAT_AUDIO_WAV       */ QT_TRANSLATE_NOOP("wdgSettingsRecording", "WAV Audio (*.wav)"),
//: Do not translate file extensions contained between parentheses [example: (*.mp3)]
/* REC_FORMAT_AUDIO_MP3       */ QT_TRANSLATE_NOOP("wdgSettingsRecording", "MP3 Audio (*.mp3)"),
//: Do not translate file extensions contained between parentheses [example: (*.mp3)]
/* REC_FORMAT_AUDIO_AAC       */ QT_TRANSLATE_NOOP("wdgSettingsRecording", "AAC Audio (*.aac)"),
//: Do not translate file extensions contained between parentheses [example: (*.mp3)]
/* REC_FORMAT_AUDIO_FLAC      */ QT_TRANSLATE_NOOP("wdgSettingsRecording", "Flac Audio (*.flac)"),
//: Do not translate file extensions contained between parentheses [example: (*.mp3)]
/* REC_FORMAT_AUDIO_OGG       */ QT_TRANSLATE_NOOP("wdgSettingsRecording", "Ogg Audio (*.ogg)"),
//: Do not translate file extensions contained between parentheses [example: (*.mp3)]
/* REC_FORMAT_AUDIO_OPUS      */ QT_TRANSLATE_NOOP("wdgSettingsRecording", "Opus Audio (*.opus)"),
};
#endif

wdgSettingsRecording::wdgSettingsRecording(QWidget *parent) : QWidget(parent) {
	setupUi(this);

	setFocusProxy(comboBox_Output_Audio_Format);

#if defined (WITH_FFMPEG)
	output_format_init();

	widget_Output_Quality->setStyleSheet(button_stylesheet());

	cfg->recording.output_custom_w = output_custom_control(cfg->recording.output_custom_w, 256, 2048, 512);
	cfg->recording.output_custom_h = output_custom_control(cfg->recording.output_custom_h, 240, 2048, 480);

	connect(comboBox_Output_Video_Format, SIGNAL(activated(int)), this, SLOT(s_output_video_format(int)));
	connect(comboBox_Output_Audio_Format, SIGNAL(activated(int)), this, SLOT(s_output_audio_format(int)));

	pushButton_Output_Quality_low->setProperty("mtype", QVariant(REC_QUALITY_LOW));
	pushButton_Output_Quality_medium->setProperty("mtype", QVariant(REC_QUALITY_MEDIUM));
	pushButton_Output_Quality_high->setProperty("mtype", QVariant(REC_QUALITY_HIGH));

	connect(pushButton_Output_Quality_low, SIGNAL(toggled(bool)), this, SLOT(s_output_quality(bool)));
	connect(pushButton_Output_Quality_medium, SIGNAL(toggled(bool)), this, SLOT(s_output_quality(bool)));
	connect(pushButton_Output_Quality_high, SIGNAL(toggled(bool)), this, SLOT(s_output_quality(bool)));

	lineEdit_Output_Custom_Width->setValidator(new QIntValidator(0, 9999, this));
	lineEdit_Output_Custom_Height->setValidator(new QIntValidator(0, 9999, this));

	connect(comboBox_Output_Resolution, SIGNAL(activated(int)), this, SLOT(s_output_resolution(int)));
	connect(lineEdit_Output_Custom_Width, SIGNAL(editingFinished()), SLOT(s_output_custom_width()));
	connect(lineEdit_Output_Custom_Height, SIGNAL(editingFinished()), SLOT(s_output_custom_height()));
	connect(checkBox_Use_emu_resolution, SIGNAL(stateChanged(int)), this, SLOT(s_use_emu_resolution(int)));
	connect(checkBox_Follow_rotation, SIGNAL(stateChanged(int)), this, SLOT(s_follow_rotation(int)));
#endif
}
wdgSettingsRecording::~wdgSettingsRecording() = default;

#if defined (WITH_FFMPEG)
void wdgSettingsRecording::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}
void wdgSettingsRecording::showEvent(UNUSED(QShowEvent *event)) {
	int dim = fontMetrics().height();

	icon_Audio_recording_settings->setPixmap(QIcon(":/icon/icons/microphone.svgz").pixmap(dim, dim));
	icon_Output_Audio_Format->setPixmap(QIcon(":/icon/icons/nsf_file.svgz").pixmap(dim, dim));
	icon_Video_recording_settings->setPixmap(QIcon(":/icon/icons/camera.svgz").pixmap(dim, dim));
	icon_Output_Video_Format->setPixmap(QIcon(":/icon/icons/film.svgz").pixmap(dim, dim));
	icon_Output_Quality->setPixmap(QIcon(":/icon/icons/recording_quality.svgz").pixmap(dim, dim));
	icon_Output_Resolution->setPixmap(QIcon(":/icon/icons/resolution.svgz").pixmap(dim, dim));
	icon_Output_Custom_Width->setPixmap(QIcon(":/icon/icons/width.svgz").pixmap(dim, dim));
	icon_Output_Custom_Height->setPixmap(QIcon(":/icon/icons/height.svgz").pixmap(dim, dim));

	update_widget();
}

void wdgSettingsRecording::retranslateUi(QWidget *wdgSettingsRecording) {
	Ui::wdgSettingsRecording::retranslateUi(wdgSettingsRecording);
	output_format_init();
	update_widget();
}
void wdgSettingsRecording::update_widget(void) {
	bool mode = true;

	if (!recording_format_info[cfg->recording.audio_format].present) {
		cfg->recording.audio_format = REC_FORMAT_AUDIO_WAV;
	}
	if (!recording_format_info[cfg->recording.video_format].present) {
		cfg->recording.video_format = REC_FORMAT_VIDEO_MPG_MPEG1;
	}

	switch (cfg->recording.video_format) {
		default:
			break;
		case REC_FORMAT_VIDEO_AVI_FFV:
		case REC_FORMAT_VIDEO_AVI_RAW:
			mode = false;
			break;
	}

	output_quality_set();

	icon_Output_Quality->setEnabled(mode);
	label_Output_Quality->setEnabled(mode);
	widget_Output_Quality->setEnabled(mode);

	comboBox_Output_Video_Format->setCurrentIndex(cfg->recording.video_format);

	output_resolution_init();

	icon_Output_Resolution->setEnabled(!cfg->recording.use_emu_resolution);
	label_Output_Resolution->setEnabled(!cfg->recording.use_emu_resolution);
	comboBox_Output_Resolution->setEnabled(!cfg->recording.use_emu_resolution);

	if (cfg->recording.use_emu_resolution) {
		mode = false;
	} else {
		mode = cfg->recording.output_resolution == REC_RES_CUSTOM;
	}

	icon_Output_Custom_Width->setEnabled(mode);
	label_Output_Custom_Width->setEnabled(mode);
	lineEdit_Output_Custom_Width->setEnabled(mode);
	lineEdit_Output_Custom_Width->setText(QString("%1").arg(cfg->recording.output_custom_w));

	icon_Output_Custom_Height->setEnabled(mode);
	label_Output_Custom_Height->setEnabled(mode);
	lineEdit_Output_Custom_Height->setEnabled(mode);
	lineEdit_Output_Custom_Height->setText(QString("%1").arg(cfg->recording.output_custom_h));

	qtHelper::checkbox_set_checked(checkBox_Use_emu_resolution, cfg->recording.use_emu_resolution);
	qtHelper::checkbox_set_checked(checkBox_Follow_rotation, cfg->recording.follow_rotation);
}

void wdgSettingsRecording::combobox_format_init(QComboBox *cb,  int start, int end) {
	enum recording_format rfstart = (enum recording_format)start;
	enum recording_format rfend = (enum recording_format)end;
	QStandardItemModel *model;
	int i;

	cb->clear();

	for (i = rfstart; i < rfend; i++) {
		_recording_format_info *rfi = &recording_format_info[i];
		QString description = tr(format_description[i]);

		if (!rfi->present) {
			description += " [" + tr("Not supported") + "]";
		}

		cb->addItem(description);
	}

	model = (QStandardItemModel *)cb->model();

	for (i = rfstart; i < rfend; i++) {
		_recording_format_info *rfi = &recording_format_info[i];
		QStandardItem *item = model->item(i - rfstart);

		if (!rfi->present) {
			item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
		}
	}
}
void wdgSettingsRecording::output_format_init(void) {
	combobox_format_init(comboBox_Output_Audio_Format, REC_FORMAT_AUDIO_WAV, REC_FORMAT_AUDIO_TOTAL);
	combobox_format_init(comboBox_Output_Video_Format, REC_FORMAT_VIDEO_MPG_MPEG1, REC_FORMAT_VIDEO_TOTAL);

	comboBox_Output_Audio_Format->setCurrentIndex(cfg->recording.audio_format - REC_FORMAT_AUDIO_WAV);
	comboBox_Output_Video_Format->setCurrentIndex(cfg->recording.video_format);
}
void wdgSettingsRecording::output_resolution_init(void) {
	int w = -1, h = -1;

	recording_decode_output_resolution(&w, &h);

	if ((w == 0) && (h == 0)) {
		comboBox_Output_Resolution->setCurrentIndex(0);
	} else {
		int index = comboBox_Output_Resolution->findText(QString("%1 x %2").arg(w).arg(h), Qt::MatchStartsWith);

		if (index != -1) {
			comboBox_Output_Resolution->setCurrentIndex(index);
		}
	}
}
void wdgSettingsRecording::output_quality_set(void) {
	qtHelper::pushbutton_set_checked(pushButton_Output_Quality_low, false);
	qtHelper::pushbutton_set_checked(pushButton_Output_Quality_medium, false);
	qtHelper::pushbutton_set_checked(pushButton_Output_Quality_high, false);
	switch (cfg->recording.quality) {
		default:
		case REC_QUALITY_LOW:
			qtHelper::pushbutton_set_checked(pushButton_Output_Quality_low, true);
			break;
		case REC_QUALITY_MEDIUM:
			qtHelper::pushbutton_set_checked(pushButton_Output_Quality_medium, true);
			break;
		case REC_QUALITY_HIGH:
			qtHelper::pushbutton_set_checked(pushButton_Output_Quality_high, true);
			break;
	}
}
int wdgSettingsRecording::output_custom_control(int actual, int min, int max, int def) {
	actual = (actual /  2) * 2;
	if ((actual < min) || (actual > max)) {
		return (def);
	}
	return (actual);
}

void wdgSettingsRecording::s_output_audio_format(int index) {
	cfg->recording.audio_format = index + REC_FORMAT_AUDIO_WAV;
	update_widget();
}
void wdgSettingsRecording::s_output_video_format(int index) {
	cfg->recording.video_format = index + REC_FORMAT_VIDEO_MPG_MPEG1;
	update_widget();
}
void wdgSettingsRecording::s_output_quality(bool checked) {
	if (checked) {
		int quality = QVariant(((QPushButton *)sender())->property("mtype")).toInt();

		cfg->recording.quality = quality;
	}
	output_quality_set();
}
void wdgSettingsRecording::s_output_resolution(int index) {
	if (index == 0) {
		cfg->recording.output_resolution = REC_RES_CUSTOM;
	} else {
		QStringList slist = ((QStandardItemModel *)comboBox_Output_Resolution->model())->item(index)->text().split(" ");
		QString wxh = slist[0] + slist[1] + slist[2];
		unsigned int i;

		for (i = 0; i < LENGTH(opt_recording_output_resolution); i++) {
			_opt set = opt_recording_output_resolution[i];

			if (wxh == uQString(set.sname)) {
				cfg->recording.output_resolution = set.value;
				break;
			}
		}
	}
	update_widget();
}
void wdgSettingsRecording::s_output_custom_width(void) {
	int w = lineEdit_Output_Custom_Width->text().toInt();

	cfg->recording.output_custom_w = output_custom_control(w, 256, 2048, cfg->recording.output_custom_w);
	update_widget();
}
void wdgSettingsRecording::s_output_custom_height(void) {
	int h = lineEdit_Output_Custom_Height->text().toInt();

	cfg->recording.output_custom_h = output_custom_control(h, 240, 2048, cfg->recording.output_custom_h);
	update_widget();
}
void wdgSettingsRecording::s_use_emu_resolution(UNUSED(int state)) {
	cfg->recording.use_emu_resolution = !cfg->recording.use_emu_resolution;
	update_widget();
}
void wdgSettingsRecording::s_follow_rotation(UNUSED(int state)) {
	cfg->recording.follow_rotation = !cfg->recording.follow_rotation;
}

// ----------------------------------------------------------------------------------------------

wdgRecGetSaveFileName::wdgRecGetSaveFileName(QWidget *parent) : QFileDialog(parent) {
	label_Output_Quality = nullptr;
	comboBox_Output_Quality = nullptr;

	rec_cfg.audio_format = cfg->recording.audio_format;
	rec_cfg.video_format = cfg->recording.video_format;
	rec_cfg.quality = cfg->recording.quality;

	setOption(QFileDialog::DontUseNativeDialog);
	setAcceptMode(QFileDialog::AcceptSave);
	setFileMode(QFileDialog::AnyFile);
	setViewMode(QFileDialog::Detail);
}
wdgRecGetSaveFileName::~wdgRecGetSaveFileName() = default;

QString wdgRecGetSaveFileName::audio_get_save_file_name(void) {
	QComboBox *cb_file_types;

	setWindowTitle(tr("Record AUDIO on file"));
	// e' importante inserire prima il nome del file ...
	selectFile(uQString(info.rom.file));
	// ... e poi la directory. Facendo il contrario imposterebbe come directory
	// il path precedente il nome (quantomeno capita nella versione windows).
	if (ustrlen(cfg->last_rec_audio_path) == 0) {
		setDirectory(QFileInfo(uQString(info.rom.file)).dir().absolutePath());
	} else {
		setDirectory(uQString(cfg->last_rec_audio_path));
	}

	if ((cb_file_types = init_file_types(REC_FORMAT_AUDIO_WAV, REC_FORMAT_AUDIO_TOTAL, rec_cfg.audio_format))) {
		connect(cb_file_types, SIGNAL(activated(int)), this, SLOT(s_output_audio_format(int)));
	}

	if (exec() == QDialog::Accepted) {
		cfg->recording.audio_format = rec_cfg.audio_format;
		return (control_filename(rec_cfg.audio_format));
	}

	return (QString());
}
QString wdgRecGetSaveFileName::video_get_save_file_name(void) {
	QGridLayout *main_layout = dynamic_cast<QGridLayout *>(layout());
	int num_rows = main_layout->rowCount();
	QComboBox *cb_file_types;

	label_Output_Quality = new QLabel(this);
	label_Output_Quality->setObjectName("label_Output_Quality");
	label_Output_Quality->setText(tr("Output Quality") + ":");
	main_layout->addWidget(label_Output_Quality, num_rows, 0, 1, 1);

	comboBox_Output_Quality = new QComboBox(this);
	comboBox_Output_Quality->setObjectName("comboBox_Output_Quality");
	comboBox_Output_Quality->addItem(tr("Low"));
	comboBox_Output_Quality->addItem(tr("Medium"));
	comboBox_Output_Quality->addItem(tr("High"));
	comboBox_Output_Quality->setCurrentIndex(rec_cfg.quality);
	main_layout->addWidget(comboBox_Output_Quality, num_rows, 1, 1, 1);

	setWindowTitle(tr("Record VIDEO on file"));
	// e' importante inserire prima il nome del file ...
	selectFile(uQString(info.rom.file));
	// ... e poi la directory. Facendo il contrario imposterebbe come directory
	// il path precedente il nome (quantomeno capita nella versione windows).
	if (ustrlen(cfg->last_rec_video_path) == 0) {
		setDirectory(QFileInfo(uQString(info.rom.file)).dir().absolutePath());
	} else {
		setDirectory(uQString(cfg->last_rec_video_path));
	}

	if ((cb_file_types = init_file_types(REC_FORMAT_VIDEO_MPG_MPEG1, REC_FORMAT_VIDEO_TOTAL, rec_cfg.video_format))) {
		connect(cb_file_types, SIGNAL(activated(int)), this, SLOT(s_output_video_format(int)));
	}
	connect(comboBox_Output_Quality, SIGNAL(activated(int)), this, SLOT(s_output_quality(int)));

	s_output_video_format(rec_cfg.video_format - REC_FORMAT_VIDEO_MPG_MPEG1);

	if (exec() == QDialog::Accepted) {
		cfg->recording.video_format = rec_cfg.video_format;
		cfg->recording.quality = rec_cfg.quality;
		gui_update_recording_tab();
		return (control_filename(rec_cfg.video_format));
	}

	return (QString());
}

QComboBox *wdgRecGetSaveFileName::init_file_types(int start, int end, int current) {
	enum recording_format rfstart = (enum recording_format)start;
	enum recording_format rfend = (enum recording_format)end;
	QList<QComboBox *>cb_list= findChildren<QComboBox *>();
	QStandardItemModel *model;
	QComboBox *cb_file_types = nullptr;
	QStringList filters;
	int i;

	for (i = rfstart; i < rfend; i++) {
		QString description = QCoreApplication::translate("wdgSettingsRecording", format_description[i]);
		_recording_format_info *rfi = &recording_format_info[i];

		if (!rfi->present) {
			description += " [" + tr("Not supported") + "]";
		}

		filters.append(description);
	}
	setNameFilters(filters);

	foreach (const QComboBox *cb, cb_list) {
		QString description = QCoreApplication::translate("wdgSettingsRecording", format_description[rfstart]);

		model = (QStandardItemModel *)cb->model();

		if (model->item(0) &&  (model->item(0)->text() == description)) {
			cb_file_types = (QComboBox *)cb;
			break;
		}
	}

	if (cb_file_types) {
		for (i = rfstart; i < rfend; i++) {
			_recording_format_info *rfi = &recording_format_info[i];
			QStandardItem *item = model->item(i - rfstart);

			if (!rfi->present) {
				item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
			}
		}
	}

	selectNameFilter(QCoreApplication::translate("wdgSettingsRecording", format_description[current]));

	return (cb_file_types);
}
QString wdgRecGetSaveFileName::control_filename(int current) {
	_recording_format_info *rfi = &recording_format_info[current];
	QFileInfo fi = QFileInfo(selectedFiles().value(0));
	char *csuffix = rfi->suffix_list[0];
	bool found = false;
	int i = 0;

	while (strcmp(csuffix, "end") != 0) {
		QString suffix = QString(csuffix);

		if (suffix.toLower() == fi.suffix().toLower()) {
			found = true;
		}
		csuffix = rfi->suffix_list[++i];
	}

	if (!found) {
		return (fi.absoluteFilePath() + "." + QString(rfi->suffix_list[0]));
	}
	return (selectedFiles().value(0));
}

void wdgRecGetSaveFileName::s_output_audio_format(int index) {
	rec_cfg.audio_format = index + REC_FORMAT_AUDIO_WAV;
}
void wdgRecGetSaveFileName::s_output_video_format(int index) {
	bool quality = true;

	rec_cfg.video_format = index + REC_FORMAT_VIDEO_MPG_MPEG1;

	switch (rec_cfg.video_format) {
		default:
			break;
		case REC_FORMAT_VIDEO_AVI_FFV:
		case REC_FORMAT_VIDEO_AVI_RAW:
			quality = false;
			break;
	}

	label_Output_Quality->setEnabled(quality);
	comboBox_Output_Quality->setEnabled(quality);
}
void wdgRecGetSaveFileName::s_output_quality(int index) {
	rec_cfg.quality = index;
}
#endif
