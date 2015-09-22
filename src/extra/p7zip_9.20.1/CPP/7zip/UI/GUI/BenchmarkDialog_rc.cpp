// BenchmarkDialog.cpp

#include "StdAfx.h"

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
 
#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif  

#undef _WIN32

#include "Windows/Control/DialogImpl.h"
#include "BenchmarkDialogRes.h"

#if 0


IDD_DIALOG_BENCHMARK DIALOG 0, 0, xSize, ySize MY_MODAL_DIALOG_STYLE | WS_MINIMIZEBOX
CAPTION "Benchmark"
MY_FONT
BEGIN
OK  PUSHBUTTON  "&Restart", IDC_BUTTON_RESTART, bXPos1, marg, bXSize, bYSize
OK  PUSHBUTTON  "&Stop",    IDC_BUTTON_STOP,    bXPos1,   27, bXSize, bYSize
  
  PUSHBUTTON  "&Help",    IDHELP,             bXPos2, bYPos, bXSize,bYSize
  PUSHBUTTON  "Cancel",   IDCANCEL,           bXPos1, bYPos, bXSize, bYSize
  
OK  LTEXT     "&Dictionary size:", IDC_BENCHMARK_DICTIONARY, marg, marg + 1, g0XSize, 8
OK  COMBOBOX  IDC_BENCHMARK_COMBO_DICTIONARY, g1XPos, marg, g1XSize, 140, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP

  LTEXT     "&Number of CPU threads:", IDC_BENCHMARK_NUM_THREADS, marg, 24, g0XSize, 8
  COMBOBOX  IDC_BENCHMARK_COMBO_NUM_THREADS, g1XPos, 23, g1XSize, 140, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP

  LTEXT     "Memory usage:", IDC_BENCHMARK_MEMORY, gc2XPos, marg + 1, gc2XSize, 8
  LTEXT     "0 MB", IDC_BENCHMARK_MEMORY_VALUE, gc2XPos + gc2XSize, marg + 1, 40, 8
  LTEXT     "1", IDC_BENCHMARK_HARDWARE_THREADS, gc2XPos, 24, 40, 8

  RTEXT     "CPU Usage", IDC_BENCHMARK_USAGE_LABEL,           gUsagePos,  53, gUsageSize,  8
  RTEXT     "Speed", IDC_BENCHMARK_SPEED_LABEL,               gSpeedPos,  53, gSpeedSize,  8
  RTEXT     "Rating / Usage", IDC_BENCHMARK_RPU_LABEL,          gRpuPos,  53, gRpuSize,    8
  RTEXT     "Rating", IDC_BENCHMARK_RATING_LABEL,            gRatingPos,  53, gRatingSize, 8
  
  GROUPBOX  "Compressing", IDC_BENCHMARK_COMPRESSING,              marg,  64,      xSize2, 40
 
  LTEXT     "Current", IDC_BENCHMARK_CURRENT,                   g10XPos,  76, gLabelSize,  8
  RTEXT     "100%", IDC_BENCHMARK_COMPRESSING_USAGE,          gUsagePos,  76, gUsageSize,  8
  RTEXT     "100 KB/s", IDC_BENCHMARK_COMPRESSING_SPEED,      gSpeedPos,  76, gSpeedSize,  8
  RTEXT     "0", IDC_BENCHMARK_COMPRESSING_RPU,                 gRpuPos,  76, gRpuSize,    8
  RTEXT     "0", IDC_BENCHMARK_COMPRESSING_RATING,           gRatingPos,  76, gRatingSize, 8
  
  LTEXT     "Resulting", IDC_BENCHMARK_RESULTING,               g10XPos,  89, gLabelSize,  8
  RTEXT     "100%", IDC_BENCHMARK_COMPRESSING_USAGE2,         gUsagePos,  89, gUsageSize,  8
  RTEXT     "100 KB/s", IDC_BENCHMARK_COMPRESSING_SPEED2,     gSpeedPos,  89, gSpeedSize,  8
  RTEXT     "0", IDC_BENCHMARK_COMPRESSING_RPU2,                gRpuPos,  89, gRpuSize,    8
  RTEXT     "0", IDC_BENCHMARK_COMPRESSING_RATING2,          gRatingPos,  89, gRatingSize, 8
  
  GROUPBOX  "Decompressing", IDC_BENCHMARK_DECOMPRESSING,          marg, 111, xSize2, 40

  LTEXT     "Current", IDC_BENCHMARK_CURRENT2,                  g10XPos, 123,  gLabelSize, 8
  RTEXT     "100%", IDC_BENCHMARK_DECOMPRESSING_USAGE,        gUsagePos, 123,  gUsageSize, 8
  RTEXT     "100 KB/s", IDC_BENCHMARK_DECOMPRESSING_SPEED,    gSpeedPos, 123,  gSpeedSize, 8
  RTEXT     "0", IDC_BENCHMARK_DECOMPRESSING_RPU,               gRpuPos, 123,    gRpuSize, 8
  RTEXT     "0", IDC_BENCHMARK_DECOMPRESSING_RATING,         gRatingPos, 123, gRatingSize, 8
  
  LTEXT     "Resulting", IDC_BENCHMARK_RESULTING2,              g10XPos, 136,  gLabelSize, 8
  RTEXT     "100%", IDC_BENCHMARK_DECOMPRESSING_USAGE2,       gUsagePos, 136,  gUsageSize, 8
  RTEXT     "100 KB/s", IDC_BENCHMARK_DECOMPRESSING_SPEED2,   gSpeedPos, 136,  gSpeedSize, 8
  RTEXT     "0", IDC_BENCHMARK_DECOMPRESSING_RPU2,              gRpuPos, 136,    gRpuSize, 8
  RTEXT     "0", IDC_BENCHMARK_DECOMPRESSING_RATING2,        gRatingPos, 136, gRatingSize, 8
  
  GROUPBOX  "Total Rating", IDC_BENCHMARK_TOTAL_RATING, gTotalRatingPos, 163, gTotalRatingSize, 38
  RTEXT     "0", IDC_BENCHMARK_TOTAL_USAGE_VALUE,             gUsagePos, 181,  gUsageSize, 8
  RTEXT     "0", IDC_BENCHMARK_TOTAL_RPU_VALUE,                 gRpuPos, 181,    gRpuSize, 8
  RTEXT     "0", IDC_BENCHMARK_TOTAL_RATING_VALUE,           gRatingPos, 181, gRatingSize, 8
  
  LTEXT     "Elapsed time:", IDC_BENCHMARK_ELAPSED,    marg, 163, g2XSize, 8
  LTEXT     "Size:", IDC_BENCHMARK_SIZE,               marg, 176, g2XSize, 8
  LTEXT     "Passes:", IDC_BENCHMARK_PASSES,           marg, 189, g2XSize, 8
  RTEXT     "00:00:00", IDC_BENCHMARK_ELAPSED_VALUE, g3XPos, 163, g3XSize, 8
  RTEXT     "0", IDC_BENCHMARK_SIZE_VALUE,           g3XPos, 176, g3XSize, 8
  RTEXT     "0", IDC_BENCHMARK_PASSES_VALUE,         g3XPos, 189, g3XSize, 8
END
#endif // #if 0

class CBenchmarkDialogImpl : public NWindows::NControl::CModalDialogImpl
{
  public:
   CBenchmarkDialogImpl(NWindows::NControl::CModalDialog *dialog,wxWindow * parent , int id) : CModalDialogImpl(dialog,parent, id, wxT("Benchmark"))
  {

	wxSizer *topsizer = new wxBoxSizer(wxVERTICAL);

	wxSizer *sizerLine1 = new wxBoxSizer(wxHORIZONTAL);

	wxSizer *sizeLine1Btn = new wxBoxSizer(wxVERTICAL);
	sizeLine1Btn->Add(new wxButton(this, IDC_BUTTON_RESTART, _T("&Restart")) , 0, wxALL|wxEXPAND, 5 );
	sizeLine1Btn->Add(new wxButton(this, IDC_BUTTON_STOP, _T("&Stop")) , 0, wxALL|wxEXPAND, 5 );

	wxSizer *sizeLine1Combo = new wxBoxSizer(wxVERTICAL);

	wxComboBox * chcDicoSize = new wxComboBox(this, IDC_BENCHMARK_COMBO_DICTIONARY,
		 wxEmptyString, wxDefaultPosition, wxDefaultSize, wxArrayString(), wxCB_READONLY);

	wxComboBox * chcThread = new wxComboBox(this, IDC_BENCHMARK_COMBO_NUM_THREADS,
		 wxEmptyString, wxDefaultPosition, wxDefaultSize, wxArrayString(), wxCB_READONLY);

	sizeLine1Combo->Add(chcDicoSize , 0, wxALL, 5 );
	sizeLine1Combo->Add(chcThread , 0, wxALL, 5 );

	wxSizer *sizeLine1ComboLabel = new wxBoxSizer(wxVERTICAL);
	sizeLine1ComboLabel->Add(new wxStaticText(this, IDC_BENCHMARK_DICTIONARY, _T("&Dictionary size:")) , 1, wxALL|wxEXPAND, 5 );
	sizeLine1ComboLabel->Add(new wxStaticText(this, IDC_BENCHMARK_NUM_THREADS, _T("&Number of CPU threads:")) , 1, wxALL|wxEXPAND, 5 );

	wxSizer *sizeLine1Col3 = new wxBoxSizer(wxVERTICAL);
	sizeLine1Col3->Add(new wxStaticText(this, IDC_BENCHMARK_MEMORY, _T("Memory usage:")) , 1, wxALL|wxEXPAND, 5 );
	sizeLine1Col3->Add(new wxStaticText(this, IDC_BENCHMARK_HARDWARE_THREADS, _T("1")) , 1, wxALL|wxEXPAND, 5 );

	wxSizer *sizeLine1Col4 = new wxBoxSizer(wxVERTICAL);
	sizeLine1Col4->Add(new wxStaticText(this, IDC_BENCHMARK_MEMORY_VALUE, _T("0 MB")) , 0, wxALL|wxEXPAND, 5 );

	sizerLine1->Add(sizeLine1ComboLabel,0, wxALL|wxEXPAND, 5);
	sizerLine1->Add(sizeLine1Combo,0, wxALL|wxEXPAND, 5);
	sizerLine1->Add(sizeLine1Col3,0, wxALL|wxEXPAND, 5);
	sizerLine1->Add(sizeLine1Col4,0, wxALL|wxEXPAND, 5);
	sizerLine1->Add(sizeLine1Btn,0, wxALL|wxEXPAND, 5);

	// LABEL (copy the structure of the compressing or decompressing group

	wxStaticBoxSizer * sizerLine2 = new wxStaticBoxSizer(new wxStaticBox(this,wxID_ANY,_T("")),wxVERTICAL);
	wxSizer *sizerLabel = new wxBoxSizer(wxHORIZONTAL);
	sizerLabel->Add(new wxStaticText(this, wxID_ANY, _T(" ")) , 1, wxALL|wxEXPAND, 5 );
	sizerLabel->Add(new wxStaticText(this, IDC_BENCHMARK_SPEED_LABEL, _T("Speed")) , 1, wxALL|wxEXPAND, 5 );
	sizerLabel->Add(new wxStaticText(this, IDC_BENCHMARK_USAGE_LABEL, _T("CPU Usage")) , 1, wxALL|wxEXPAND, 5 );
	sizerLabel->Add(new wxStaticText(this, IDC_BENCHMARK_RPU_LABEL, _T("Rating / Usage")), 1, wxALL|wxEXPAND, 5 );
	sizerLabel->Add(new wxStaticText(this, IDC_BENCHMARK_RATING_LABEL, _T("Rating")) , 1, wxALL|wxEXPAND, 5 );

	sizerLine2->Add(sizerLabel, 0, wxALL|wxEXPAND, 5);

	// GROUP COMPRESSING

	wxStaticBoxSizer * grpCompress = new wxStaticBoxSizer(new wxStaticBox(this,IDC_BENCHMARK_COMPRESSING,_T("Compressing")),wxVERTICAL);
	wxSizer *grpCompress1 = new wxBoxSizer(wxHORIZONTAL);
	grpCompress1->Add(new wxStaticText(this, IDC_BENCHMARK_CURRENT, _T("Current")) , 1, wxALL|wxEXPAND, 5 );
	grpCompress1->Add(new wxStaticText(this, IDC_BENCHMARK_COMPRESSING_SPEED, _T("100 KB/s")) , 1, wxALL|wxEXPAND, 5 );
	grpCompress1->Add(new wxStaticText(this, IDC_BENCHMARK_COMPRESSING_USAGE, _T("100%")) , 1, wxALL|wxEXPAND, 5 );
	grpCompress1->Add(new wxStaticText(this, IDC_BENCHMARK_COMPRESSING_RPU, _T("0")), 1, wxALL|wxEXPAND, 5 );
	grpCompress1->Add(new wxStaticText(this, IDC_BENCHMARK_COMPRESSING_RATING, _T("0")) , 1, wxALL|wxEXPAND, 5 );

	wxSizer *grpCompress2 = new wxBoxSizer(wxHORIZONTAL);
	grpCompress2->Add(new wxStaticText(this, IDC_BENCHMARK_RESULTING, _T("Resulting")) , 1, wxALL|wxEXPAND, 5 );
	grpCompress2->Add(new wxStaticText(this, IDC_BENCHMARK_COMPRESSING_SPEED2, _T("100 KB/s")) , 1, wxALL|wxEXPAND, 5 );
	grpCompress2->Add(new wxStaticText(this, IDC_BENCHMARK_COMPRESSING_USAGE2, _T("100%")) , 1, wxALL|wxEXPAND, 5 );
	grpCompress2->Add(new wxStaticText(this, IDC_BENCHMARK_COMPRESSING_RPU2, _T("0")) , 1, wxALL|wxEXPAND, 5);
	grpCompress2->Add(new wxStaticText(this, IDC_BENCHMARK_COMPRESSING_RATING2, _T("0")) , 1, wxALL|wxEXPAND, 5 );

	grpCompress->Add(grpCompress1, 0, wxALL|wxEXPAND, 5);
	grpCompress->Add(grpCompress2, 0, wxALL|wxEXPAND, 5);

	// GROUP DECOMPRESSING

	wxStaticBoxSizer * grpDecompress = new wxStaticBoxSizer(new wxStaticBox(this,IDC_BENCHMARK_DECOMPRESSING,_T("Decompressing")),wxVERTICAL);
	wxSizer *grpDecompress1 = new wxBoxSizer(wxHORIZONTAL);
	grpDecompress1->Add(new wxStaticText(this, IDC_BENCHMARK_CURRENT2, _T("Current")) , 1, wxALL|wxEXPAND, 5 );
	grpDecompress1->Add(new wxStaticText(this, IDC_BENCHMARK_DECOMPRESSING_SPEED, _T("100 KB/s")) , 1, wxALL|wxEXPAND, 5 );
	grpDecompress1->Add(new wxStaticText(this, IDC_BENCHMARK_DECOMPRESSING_USAGE, _T("100%")) , 1, wxALL|wxEXPAND, 5 );
	grpDecompress1->Add(new wxStaticText(this, IDC_BENCHMARK_DECOMPRESSING_RPU, _T("0")), 1, wxALL|wxEXPAND, 5 );
	grpDecompress1->Add(new wxStaticText(this, IDC_BENCHMARK_DECOMPRESSING_RATING, _T("0")) , 1, wxALL|wxEXPAND, 5 );

	wxSizer *grpDecompress2 = new wxBoxSizer(wxHORIZONTAL);
	grpDecompress2->Add(new wxStaticText(this, IDC_BENCHMARK_RESULTING2, _T("Resulting")) , 1, wxALL|wxEXPAND, 5 );
	grpDecompress2->Add(new wxStaticText(this, IDC_BENCHMARK_DECOMPRESSING_SPEED2, _T("100 KB/s")) , 1, wxALL|wxEXPAND, 5 );
	grpDecompress2->Add(new wxStaticText(this, IDC_BENCHMARK_DECOMPRESSING_USAGE2, _T("100%")) , 1, wxALL|wxEXPAND, 5 );
	grpDecompress2->Add(new wxStaticText(this, IDC_BENCHMARK_DECOMPRESSING_RPU2, _T("0")) , 1, wxALL|wxEXPAND, 5);
	grpDecompress2->Add(new wxStaticText(this, IDC_BENCHMARK_DECOMPRESSING_RATING2, _T("0")) , 1, wxALL|wxEXPAND, 5 );

	grpDecompress->Add(grpDecompress1, 0, wxALL|wxEXPAND, 5);
	grpDecompress->Add(grpDecompress2, 0, wxALL|wxEXPAND, 5);

	// GROUPE TOTAL RATING
	wxStaticBoxSizer * grpTotalRating = new wxStaticBoxSizer(new wxStaticBox(this,IDC_BENCHMARK_TOTAL_RATING,_T("Total Rating")),wxHORIZONTAL);
	grpTotalRating->Add(new wxStaticText(this, wxID_ANY, _T("")) , 1, wxALL|wxEXPAND, 5 );
	grpTotalRating->Add(new wxStaticText(this, IDC_BENCHMARK_TOTAL_USAGE_VALUE, _T("0")) , 1, wxALL|wxEXPAND, 5 );
	grpTotalRating->Add(new wxStaticText(this, IDC_BENCHMARK_TOTAL_RPU_VALUE, _T("0")) , 1, wxALL|wxEXPAND, 5 );
	grpTotalRating->Add(new wxStaticText(this, IDC_BENCHMARK_TOTAL_RATING_VALUE, _T("0")) , 1, wxALL|wxEXPAND, 5 );

	// GROUPE ELAPSED TIME
	wxSizer * grpElapsedTime = new wxBoxSizer(wxHORIZONTAL);

	wxSizer * grpElapsedTime1 = new wxBoxSizer(wxVERTICAL);
	grpElapsedTime1->Add(new wxStaticText(this, IDC_BENCHMARK_ELAPSED, _T("Elapsed time:")) , 0, wxALL|wxEXPAND, 5 );
	grpElapsedTime1->Add(new wxStaticText(this, IDC_BENCHMARK_SIZE, _T("Size:")) , 0, wxALL|wxEXPAND, 5 );
	grpElapsedTime1->Add(new wxStaticText(this, IDC_BENCHMARK_PASSES, _T("Passes:")) , 0, wxALL|wxEXPAND, 5 );

	wxSizer * grpElapsedTime2 = new wxBoxSizer(wxVERTICAL);
	grpElapsedTime2->Add(new wxStaticText(this, IDC_BENCHMARK_ELAPSED_VALUE, _T("00:00:00")) , 0, wxALL|wxEXPAND, 5 );
	grpElapsedTime2->Add(new wxStaticText(this, IDC_BENCHMARK_SIZE_VALUE, _T("0")) , 0, wxALL|wxEXPAND, 5 );
	grpElapsedTime2->Add(new wxStaticText(this, IDC_BENCHMARK_PASSES_VALUE, _T("0")) , 0, wxALL|wxEXPAND, 5 );

	grpElapsedTime->Add(grpElapsedTime1,0, wxALL|wxEXPAND, 5);
	grpElapsedTime->Add(grpElapsedTime2,0, wxALL|wxEXPAND, 5);

	wxSizer * grp_ElapsedTime_TotalRating = new wxBoxSizer(wxHORIZONTAL);
	grp_ElapsedTime_TotalRating->Add(grpElapsedTime, 0, wxALL|wxEXPAND, 5);
	grp_ElapsedTime_TotalRating->Add(grpTotalRating, 1, wxALL|wxEXPAND, 5);

	// TOP
	topsizer->Add(sizerLine1,0, wxALL|wxEXPAND, 5);
	topsizer->Add(sizerLine2,0, wxALL|wxEXPAND, 5);
	topsizer->Add(grpCompress, 0, wxALL|wxEXPAND, 5);
	topsizer->Add(grpDecompress, 0, wxALL|wxEXPAND, 5);
	topsizer->Add(grp_ElapsedTime_TotalRating, 0, wxALL|wxEXPAND, 5);

	topsizer->Add(CreateButtonSizer(wxHELP|wxCANCEL), 0, wxALL|wxEXPAND, 5);

	this->OnInit();

	SetSizer(topsizer); // use the sizer for layout
	topsizer->SetSizeHints(this); // set size hints to honour minimum size
  }
private:
	// Any class wishing to process wxWindows events must use this macro
	DECLARE_EVENT_TABLE()
};

REGISTER_DIALOG(IDD_DIALOG_BENCHMARK,CBenchmarkDialog,0)

// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(CBenchmarkDialogImpl, wxDialog)
	EVT_TIMER(wxID_ANY, CModalDialogImpl::OnAnyTimer)
	EVT_BUTTON(wxID_ANY, CModalDialogImpl::OnAnyButton)
	EVT_COMBOBOX(wxID_ANY,    CModalDialogImpl::OnAnyChoice)
	EVT_MENU(WORKER_EVENT, CModalDialogImpl::OnWorkerEvent)
END_EVENT_TABLE()

