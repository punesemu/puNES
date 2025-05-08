/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#include <QtCore/QTimer>
#include <QtCore/QList>
#include <QtGui/QScreen>
#include "mainWindow.hpp"
#include "dlgDetachBarcode.hpp"
#include "gui.h"
#include "info.h"
#include "detach_barcode.h"

dlgDetachBarcode::name_detach_barcode cardsDefault = {
	{ "",												"" }
};
dlgDetachBarcode::name_detach_barcode cardsDragonBallZ = {
	{ "",												"" },
	{ "Character Card: 人造人間16",						"0020502044333" },
	{ "Character Card: 人造人間17",						"0040416100376" },
	{ "Character Card: 人造人間18",						"0062446208037" },
	{ "Character Card: 人造人間19",						"0040600205054" },
	{ "Character Card: ギニュー",						"0088658921113" },
	{ "Character Card: セル",							"0000636053258" },
	{ "Character Card: 餃子",							"0020850515295" },
	{ "Character Card: フリーザ",						"0000616107230" },
	{ "Character Card: 悟飯",							"0002852100013" },
	{ "Character Card: クリリン",						"0088640910255" },
	{ "Character Card: ピッコロ",						"0088302101212" },
	{ "Character Card: ラティッツ",						"0048734105257" },
	{ "Character Card: サイバイマン",						"0004732055154" },
	{ "Character Card: 孫 悟空",							"0022248300117" },
	{ "Character Card: スーパーサイヤ人　べジータ",			"0022738106373" },
	{ "Character Card: 天津飯",							"0000210303212" },
	{ "Character Card: スーパーサイヤ人　トランクス",		"0062982144233" },
	{ "Character Card: べジータ",						"0068400144373" },
	{ "Character Card: ヤムチャ",						"0020242205254" },
	{ "Character Card: ザーボン",						"0044444185152" },
	{ "Special Character Card: スーパーサイヤ人　孫 悟空",	"9210340180138" },
	{ "Item Card: バトルスーツ",							"0162347145254" },
	{ "Item Card: バブルス",								"0120675202216" },
	{ "Item Card: ブルマ",								"0140601201052" },
	{ "Item Card: ブルマの母",							"0140643200013" },
	{ "Item Card: チチ",									"0162747144253" },
	{ "Item Card: ブリーフ博士",							"0120637202254" },
	{ "Item Card: ドラゴンボール",							"0162741145218" },
	{ "Item Card: 回復カプセル",							"0122815544019" },
	{ "Item Card: カリン様",								"0120631203219" },
	{ "Item Card: エンマ大王",							"0120671203255" },
	{ "Item Card: ミスターポポ",							"0162415545016" },
	{ "Item Card: ウーロン",								"0140645200059" },
	{ "Item Card: 牛魔王",								"0162453544057" },
	{ "Item Card: ポルンガ",								"0162413545018" },
	{ "Item Card: 亀仙人",								"0120215307258" }
};
dlgDetachBarcode::name_detach_barcode cardsUltramanClub = {
	{ "",												"" },
	{ "Character Card: ゾフィー",						"0315424322677" },
	{ "Character Card: ウルトラマン ジャック",				"0344046250372" },
	{ "Character Card: ウルトラマン",						"0310026276212" },
	{ "Character Card: ウルトラマンタロウ",				"0304340013455" },
	{ "Character Card: ウルトラマンアース",				"0340506071631" },
	{ "Character Card: ウルトラマンレオ",					"0354562352030" },
	{ "Character Card: ベムスター",						"0370211242217" },
	{ "Character Card: アストラ",						"0301546152452" },
	{ "Character Card: ウルトラセブン",					"0354402330518" },
	{ "Character Card: シーボーズ",						"0344543150519" },
	{ "Character Card: カネゴン",						"0341125655455" },
	{ "Character Card: アントラー",						"0313420753129" },
	{ "Character Card: バルタン星人",						"0311027362416" },
	{ "Character Card: エレキング",						"0351423042543" },
	{ "Character Card: ぺスター",						"0300137370060" },
	{ "Character Card: シーゴラス",						"0352030276123" },
	{ "Character Card: ゴモラ",							"0302406330614" },
	{ "Character Card: ダダ",							"0351065350365" },
	{ "Character Card: グドン",							"0353404234466" },
	{ "Character Card: レッドキング",						"0340503173222" },
	{ "Character Card: タッコング",						"0362010212737" },
	{ "Character Card: シーモンス",						"0360115332657" },
	{ "Character Card: ピグモン",						"0354147246266" },
	{ "Character Card: ゼットン",						"0315005265225" },
	{ "Character Card: ウー",							"0443104216678" },
	{ "Character Card: ツインテール",						"0313612255073" },
	{ "Character Card: メトロン星人",						"0360003131751" },
	{ "Item Card: ガヴァドン",							"0452177111759" },
	{ "Item Card: ウルトラの父",							"0416434374356" },
	{ "Item Card: ペギラ",								"0407477371723" },
	{ "Item Card: ネロンガ",								"0403075341410" },
	{ "Item Card: ガンダー",								"0452435231007" },
	{ "Item Card: ミクラス",								"0453534350279" },
	{ "Item Card: スカイドン",							"0406143256272" },
	{ "Item Card: パンドン",								"0452114250619" },
	{ "Item Card: エンマーゴ",							"0436134251061" },
	{ "Item Card: ペガッサ星人",							"0406525340223" },
	{ "Item Card: ナックル星人",							"0456173047067" }
};
dlgDetachBarcode::name_detach_barcode cardsSDGundamWars = {
	{"",												""},
	{"Character Card: グンダム",							"0403775140252"},
	{"Character Card: グンダムGP01F/b",					"0403131741079"},
	{"Character Card: ジムカスタム",						"0401001501112"},
	{"Character Card: ジムキャノン II",					"0402131502024"},
	{"Character Card: Zグンダム",						"0401046744055"},
	{"Character Card: リックディアス",						"0403001143149"},
	{"Character Card: ZZグンダム",						"0407114541014"},
	{"Character Card: ν・グンダム",						"0401514545108"},
	{"Character Card: グンダムF-91",						"0403554500147"},
	{"Character Card: ザク II",							"0401001150105"},
	{"Character Card: ドム",								"0403001106151"},
	{"Character Card: ゲルググ",							"0402130101068"},
	{"Character Card: ジオング",							"0401000506156"},
	{"Character Card: グフ",								"0401031441006"},
	{"Character Card: ズゴック",							"0402101145572"},
	{"Character Card: ハイザック",						"0403131400075"},
	{"Character Card: メッサーラ",						"0400530541019"},
	{"Character Card: パラス・アテネ",						"0401540442532"},
	{"Character Card: キュベレイ",						"0401030142010"},
	{"Character Card: ドライセン",						"0401407405045"},
	{"Character Card: ヤクト・ドーガ",						"0403500100568"},
	{"Character Card: サザビー",							"0400440140142"},
	{"Character Card: ベルガ・ギロス",						"0402102142020"},
	{"Character Card: ビギナ・ギナ",						"0403450442138"},
	{"Character Card: デナン・ゾン",						"0401115401056"},
	{"Character Card: グンダム Mk-II",					"0402733635151"},
	{"Character Card: 百式",								"0402434441006"},
	{"Character Card: ベルガ・ダラス",						"0402500402160"},
	{"Character Card: シャア専用ザク",						"0400741404172"},
	{"Character Card: エルメス",							"0400441406148"},
	{"Character Card: ギャン",							"0403503043008"},
	{"Character Card: ガルバルディ・β",					"0401145042557"},
	{"Character Card: ジ・O",							"0402450400513"},
	{"Character Card: ハンマ・ハンマ",						"0400144403109"},
	{"Character Card: バウ",								"0401142240062"},
	{"Character Card: ザク III",							"0406154264044"},
	{"Character Card: ドーベンウルフ",						"0402440042532"},
	{"Character Card: ガンダムGP-03",					"9346126717639"},
	{"Command Card: ホワイトベース",						"0465464360068"},
	{"Command Card: プロペラントタンク",					"0420416654671"},
	{"Command Card: ソーラシステム",						"0440302250565"},
	{"Command Card: 弾幕を張れ!",							"0443304064666"},
	{"Command Card: ガンペリー",							"0447363361032"},
	{"Command Card: ミデア",								"0460553366228"},
	{"Command Card: フルアーマーユニット",					"0420002452667"},
	{"Command Card: コアブースター",						"0464121725370"},
	{"Command Card: だいじょうぶ",						"0425122476371"},
	{"Command Card: コロニー落とし",						"0443327461770"},
	{"Command Card: ジェットストリーム",					"0420516274656"},
	{"Command Card: ビグザム",							"0444551077331"},
	{"Command Card: サイコフレーム",						"0462523640710"},
	{"Command Card: ガウ爆撃!",							"0464552366074"},
	{"Command Card: ヴァル・ヴァロ",						"0441351472243"},
	{"Command Card: α・アジール",							"0427627620136"},
	{"Command Card: 先手必勝!",							"0447220635665"},
	{"Command Card: みんなつっこめ!",						"0467760762065"},
	{"Command Card: たのむ行ってくれ!",					"0445726750448"},
	{"Command Card: オレが行く!",							"0467061567000"},
	{"Command Card: みんな撃て!",							"0401544774318"},
	{"Command Card: たのむ撃ってくれ!",					"0405564736646"},
	{"Command Card: みんな突進!",							"0462730572521"},
	{"Command Card: 真っ向勝負!!",						"0466765740016"},
	{"Command Card: ザコをねらえ!",						"0420434751369"},
	{"Command Card: 援護ミサイル",						"0427062053452"},
	{"Command Card: メガバズーカランチャー",				"0443513666064"},
	{"Command Card: めくらまし",							"0466120475669"},
	{"Command Card: ラフレシア",							"0426064513261"},
	{"Command Card: サイコグンダム",						"0427167436709"},
	{"Command Card: 突攻",								"0404227163515"},
	{"Command Card: さきにやっちゃまえ!",					"0440730573106"},
	{"Command Card: エースをねらえ!",						"0464524756476"},
	{"Command Card: エースを斬れ!",						"0426464772275"},
	{"Command Card: ザコをちらせ!",						"0402743543040"},
	{"Command Card: 守りを堅めろ!",						"0405645155656"},
	{"Command Card: オレが撃つ!",							"0403007706119"},
	{"Command Card: デンドロビウム",						"9366723460578"}
};

dlgDetachBarcode::dlgDetachBarcode(QWidget *parent) : QDialog(parent) {
	setupUi(this);

	setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
	setAttribute(Qt::WA_DeleteOnClose);

	ndb = cardsDefault;

	connect(listWidget_Barcodes, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(s_barcode_click(QListWidgetItem*)));
	connect(listWidget_Barcodes, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(s_barcode_doubleclick(QListWidgetItem*)));
	connect(pushButton_Apply, SIGNAL(clicked(bool)), this, SLOT(s_apply_clicked(bool)));

	adjustSize();

	{
		QMargins vgbm = verticalLayout_groupBox_Detach_Barcode->contentsMargins();
		QMargins vdia = verticalLayout_Detach_Barcode->contentsMargins();
		themePushButton *close = new themePushButton(this);
		int x = 0, y = 0, w = 0, h = 0;

		w = close->fontMetrics().size(0, "x").width() + 10;
		h = close->fontMetrics().size(0, "x").height() + 5;
		x = normalGeometry().width() - w - vdia.right() - 2 - vgbm.right();
		y = vdia.top() + 2 + 1;

		close->setGeometry(x, y, w, h);
		close->setText("x");

		connect(close, SIGNAL(clicked(bool)), this, SLOT(s_x_clicked(bool)));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		vgbm.setTop(close->sizeHint().height() + 2);
		verticalLayout_groupBox_Detach_Barcode->setContentsMargins(vgbm);
#endif
	}

	installEventFilter(this);
}
dlgDetachBarcode::~dlgDetachBarcode() = default;

bool dlgDetachBarcode::eventFilter(QObject *obj, QEvent *event) {
	switch (event->type()) {
		case QEvent::WindowActivate:
		case QEvent::WindowDeactivate:
			gui_control_pause_bck(event->type());
			break;
		default:
			break;
	}
	return (QDialog::eventFilter(obj, event));
}
void dlgDetachBarcode::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		Ui::dlgDetachBarcode::retranslateUi(this);
		if(listWidget_Barcodes->count() > 0) {
			QListWidgetItem* item = listWidget_Barcodes->item(0);

			item->setText(tr("Enter a barcode yourself..."));
		}
	} else {
		QWidget::changeEvent(event);
	}
}

int dlgDetachBarcode::update_pos(int startY) {
	int x = parentWidget()->pos().x() + parentWidget()->frameGeometry().width();
	int y = parentWidget()->geometry().y() + startY;
	QRect g = QGuiApplication::primaryScreen()->virtualGeometry();

	if ((x + frameGeometry().width() - g.left()) > g.width()) {
		x = parentWidget()->pos().x() - frameGeometry().width();
	}
	move(QPoint(x, y));

	if (isHidden()) {
		return (0);
	}

	return (frameGeometry().height());
}
void dlgDetachBarcode::update_dialog(void) {
	groupBox_Detach_Barcode->setEnabled(detach_barcode.enabled);
}
void dlgDetachBarcode::change_rom(void) {
	ndb = cardsDefault;

	if (info.crc32.prg == 0x19E81461) {
		ndb = cardsDragonBallZ;
	} else if (info.crc32.prg == 0x5B457641) {
		ndb = cardsUltramanClub;
	} else if (info.crc32.prg == 0x0BE0A328) {
		ndb = cardsSDGundamWars;
	}
	listWidget_Barcodes->clear();
	for (int i = 0; i < ndb.count(); i++) {
		const QIcon icon = i == 0 ? QIcon(":/icon/icons/pencil.svgz") : QIcon(":/icon/icons/barcode.svgz");
		const QString desc = i == 0 ? tr("Enter a barcode yourself...") : ndb[i].name;

		listWidget_Barcodes->addItem(new QListWidgetItem(icon, desc));
	}
	if (listWidget_Barcodes->count() > 0) {
		QListWidgetItem* item = listWidget_Barcodes->item(0);

		listWidget_Barcodes->setCurrentItem(item);
		s_barcode_click(item);
	}
}

void dlgDetachBarcode::apply_barcode(void) {
	detach_barcode_bcode(uQStringCD(lineEdit_Barcode->text()));
	gui_active_window();
	gui_set_focus();
}

void dlgDetachBarcode::s_barcode_click(QListWidgetItem *item) {
	selected = listWidget_Barcodes->row(item);

	lineEdit_Barcode->setReadOnly(selected > 0);
	lineEdit_Barcode->setClearButtonEnabled(selected == 0);
	lineEdit_Barcode->setText(ndb[selected].code);
}
void dlgDetachBarcode::s_barcode_doubleclick(QListWidgetItem *item) {
	s_barcode_click(item);
	apply_barcode();
}
void dlgDetachBarcode::s_apply_clicked(UNUSED(bool checked)) {
	apply_barcode();
}
void dlgDetachBarcode::s_x_clicked(UNUSED(bool checked)) {
	mainwin->s_set_detach_barcode_window();
}