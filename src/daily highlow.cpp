#include "sierrachart.h"

SCDLLName("BREAKING DAILY HIGHLOW")

SCSFExport scsf_breaking_dh(SCStudyInterfaceRef sc)
{
    SCSubgraphRef daily_high = sc.Subgraph[0];
    SCSubgraphRef daily_low = sc.Subgraph[1];
    SCSubgraphRef mid = sc.Subgraph[2];
    SCSubgraphRef daily_low_mid = sc.Subgraph[3];
    SCSubgraphRef daily_high_mid = sc.Subgraph[4];
    SCSubgraphRef background = sc.Subgraph[5];
    SCInputRef RTH_start = sc.Input[0];
    SCInputRef RTH_end = sc.Input[1];
    SCInputRef display_values = sc.Input[2];
    SCInputRef nr_of_mins_back = sc.Input[3];

    if (sc.SetDefaults)
    {
        sc.GraphName = "Breaking Highlow";

        daily_high.Name = "Daily high";
        daily_high.DrawStyle = DRAWSTYLE_LINE;
        daily_high.PrimaryColor = RGB(0, 255, 0);
        daily_high.LineWidth = 1;
        daily_high.DrawZeros = false;

        daily_low.Name = "Daily low";
        daily_low.DrawStyle = DRAWSTYLE_LINE;
        daily_low.PrimaryColor = RGB(255, 0, 0);
        daily_low.LineWidth = 1;
        daily_low.DrawZeros = false;

        mid.Name = "Mid";
        mid.DrawStyle = DRAWSTYLE_LINE;
        mid.PrimaryColor = RGB(255, 255, 255);
        mid.LineWidth = 1;
        mid.DrawZeros = false;

        daily_low_mid.Name = "Daily low mid";
        daily_low_mid.DrawStyle = DRAWSTYLE_LINE;
        daily_low_mid.PrimaryColor = RGB(120, 120, 120);
        daily_low_mid.LineWidth = 1;
        daily_low_mid.DrawZeros = false;

        daily_high_mid.Name = "Daily high mid";
        daily_high_mid.DrawStyle = DRAWSTYLE_LINE;
        daily_high_mid.PrimaryColor = RGB(120, 120, 120);
        daily_high_mid.LineWidth = 1;
        daily_high_mid.DrawZeros = false;

        display_values.Name = "Display values?";
        display_values.SetYesNo(TRUE);

        nr_of_mins_back.Name = "How many mins back to count the break valid?";
        nr_of_mins_back.SetInt(10);

        RTH_start.Name = "RTH start";
        RTH_start.SetTime(HMS_TIME(8, 30, 0));
        RTH_end.Name = "RTH end";
        RTH_end.SetTime(HMS_TIME(15, 15, 00));

        sc.AutoLoop = 1;
        sc.GraphRegion = 0;
        sc.UpdateAlways = 1;
        sc.FreeDLL = 1;
        return;
    }

    double& perzist_daily_high = sc.GetPersistentDouble(0);
    double& perzist_daily_low = sc.GetPersistentDouble(1);
    double& perzist_mid = sc.GetPersistentDouble(2);
    double& perzist_daily_low_mid = sc.GetPersistentDouble(3);
    double& perzist_daily_high_mid = sc.GetPersistentDouble(4);

    char pv_quadrant_nr = sc.GetPersistentInt(0); //this will hold the number of quadrant where last price is

    //get time of bar that made the daily high
    auto &pv_daily_high_dt = sc.GetPersistentSCDateTime(0);
    int dh_time = pv_daily_high_dt.GetTime();
    int dh_year, dh_month, dh_day, dh_hour, dh_minute, dh_second;
    pv_daily_high_dt.GetDateTimeYMDHMS(dh_year, dh_month, dh_day, dh_hour, dh_minute, dh_second);

    //get time of bar that made the daily low
    auto &pv_daily_low_dt = sc.GetPersistentSCDateTime(1);
    int dl_year, dl_month, dl_day, dl_hour, dl_minute, dl_second;
    pv_daily_low_dt.GetDateTimeYMDHMS(dl_year, dl_month, dl_day, dl_hour, dl_minute, dl_second);

    //get time of current bar 
    auto current_bar_dt = sc.BaseDateTimeIn[sc.Index];
    int year, month, day, hour, minute, second;
    current_bar_dt.GetDateTimeYMDHMS(year, month, day, hour, minute, second);

    //get time of bar X minutes prior actual bar 
    auto prior_bar_dt = static_cast<SCDateTime>(sc.BaseDateTimeIn[sc.IndexOfLastVisibleBar] - nr_of_mins_back.GetInt()*MINUTES);
    int pb_year, pb_month, pb_day, pb_hour, pb_minute, pb_second;
    prior_bar_dt.GetDateTimeYMDHMS(pb_year, pb_month, pb_day, pb_hour, pb_minute, pb_second);
    int prior_bar_index = sc.GetNearestMatchForSCDateTime(sc.ChartNumber, prior_bar_dt);

    if (sc.Index == 0) //reset if recalculated
    {
        perzist_daily_high = sc.High[sc.Index];
        perzist_daily_low = sc.Low[sc.Index];
        perzist_mid = (sc.High[sc.Index] - sc.Low[sc.Index]) / 2 + sc.Low[sc.Index];
        perzist_daily_low_mid = (sc.High[sc.Index] - sc.Low[sc.Index]) / 4 + sc.Low[sc.Index];
        perzist_daily_high_mid = (sc.High[sc.Index] - sc.Low[sc.Index]) / 4 + 3 * sc.Low[sc.Index];

        pv_daily_high_dt = sc.BaseDateTimeIn[sc.Index];
        pv_daily_low_dt = sc.BaseDateTimeIn[sc.Index];
    }
    if (sc.BaseDateTimeIn[sc.Index].GetTime() == RTH_start.GetTime())
    {
        perzist_daily_high = sc.High[sc.Index];
        perzist_daily_low = sc.Low[sc.Index];
        perzist_mid = (sc.High[sc.Index] - sc.Low[sc.Index]) / 2 + sc.Low[sc.Index];
        perzist_daily_high_mid = (sc.High[sc.Index] - sc.Low[sc.Index]) / 4 + perzist_mid;
        perzist_daily_low_mid = (sc.High[sc.Index] - sc.Low[sc.Index]) / 4 + sc.Low[sc.Index];

        pv_daily_high_dt = sc.BaseDateTimeIn[sc.Index];
        pv_daily_low_dt = sc.BaseDateTimeIn[sc.Index];
    }
    if (sc.High[sc.Index] > perzist_daily_high) //update when going higher
    {
        perzist_daily_high = sc.High[sc.Index];
        perzist_mid = (perzist_daily_high - perzist_daily_low) / 2 + perzist_daily_low;
        perzist_daily_high_mid = (perzist_daily_high - perzist_mid) / 2 + perzist_mid;
        perzist_daily_low_mid = (perzist_mid - perzist_daily_low) / 2 + perzist_daily_low;

        pv_daily_high_dt = sc.BaseDateTimeIn[sc.Index]; //get the datetime perzist of bar that broke the lastest high
    }
    if (sc.Low[sc.Index] < perzist_daily_low) //update when going lower
    {
        perzist_daily_low = sc.Low[sc.Index];
        perzist_mid = (perzist_daily_high - perzist_daily_low) / 2 + perzist_daily_low;
        perzist_daily_high_mid = (perzist_daily_high - perzist_mid) / 2 + perzist_mid;
        perzist_daily_low_mid = (perzist_mid - perzist_daily_low) / 2 + perzist_daily_low;

        pv_daily_low_dt = sc.BaseDateTimeIn[sc.Index]; //get the datetime perzist of bar that broke the latest low
    }

    daily_high[sc.Index] = (float)perzist_daily_high;
    daily_low[sc.Index] = (float)perzist_daily_low;
    mid[sc.Index] = (float)perzist_mid;
    daily_high_mid[sc.Index] = (float)perzist_daily_high_mid;
    daily_low_mid[sc.Index] = (float)perzist_daily_low_mid;
    
    //set the quadrant pv based on actual price -- using last visible bar for debuging !!!!!!!
    if (sc.Close[sc.IndexOfLastVisibleBar] > daily_low[sc.IndexOfLastVisibleBar] && sc.Close[sc.IndexOfLastVisibleBar] <= daily_low_mid[sc.IndexOfLastVisibleBar]) pv_quadrant_nr = 1;
    else if (sc.Close[sc.IndexOfLastVisibleBar] > daily_low_mid[sc.IndexOfLastVisibleBar] && sc.Close[sc.IndexOfLastVisibleBar] <= mid[sc.IndexOfLastVisibleBar]) pv_quadrant_nr = 2;
    else if (sc.Close[sc.IndexOfLastVisibleBar] > mid[sc.IndexOfLastVisibleBar] && sc.Close[sc.IndexOfLastVisibleBar] <= daily_high_mid[sc.IndexOfLastVisibleBar]) pv_quadrant_nr = 3;
    else if (sc.Close[sc.IndexOfLastVisibleBar] > daily_high_mid[sc.IndexOfLastVisibleBar] && sc.Close[sc.IndexOfLastVisibleBar] <= daily_high[sc.IndexOfLastVisibleBar]) pv_quadrant_nr = 4;

    s_UseTool breaking_dh;
    breaking_dh.Clear();
    breaking_dh.ChartNumber = sc.ChartNumber;
    breaking_dh.DrawingType = DRAWING_TEXT;
    breaking_dh.FontSize = 15;
    breaking_dh.FontBold = TRUE;
    breaking_dh.AddMethod = UTAM_ADD_OR_ADJUST;
    breaking_dh.UseRelativeVerticalValues = 1;
    breaking_dh.Color = RGB(255, 255, 255);
    breaking_dh.Region = 0;
    breaking_dh.Text.Format("New Daily High!! Broken less than %i min ago!!", nr_of_mins_back.GetInt());
    breaking_dh.LineNumber = 10;
    breaking_dh.BeginDateTime = 55;
    breaking_dh.BeginValue = 94;
    if (daily_high[sc.IndexOfLastVisibleBar] > daily_high[prior_bar_index]) { sc.UseTool(breaking_dh); }
    else sc.DeleteACSChartDrawing(sc.ChartNumber, TOOL_DELETE_CHARTDRAWING, 10);
    if (sc.Close[sc.IndexOfLastVisibleBar] < mid[sc.IndexOfLastVisibleBar]) sc.DeleteACSChartDrawing(sc.ChartNumber, TOOL_DELETE_CHARTDRAWING, 10);

    s_UseTool breaking_dl;
    breaking_dl.Clear();
    breaking_dl.ChartNumber = sc.ChartNumber;
    breaking_dl.DrawingType = DRAWING_TEXT;
    breaking_dl.FontSize = 15;
    breaking_dl.FontBold = TRUE;
    breaking_dl.AddMethod = UTAM_ADD_OR_ADJUST;
    breaking_dl.UseRelativeVerticalValues = 1;
    breaking_dl.Color = RGB(255, 255, 255);
    breaking_dl.Region = 0;
    breaking_dl.Text.Format("New Daily Low!! Broken less than %i min ago!!", nr_of_mins_back.GetInt());
    breaking_dl.LineNumber = 11;
    breaking_dl.BeginDateTime = 55;
    breaking_dl.BeginValue = 94;
    if (daily_low[sc.IndexOfLastVisibleBar] < daily_low[prior_bar_index]) { sc.UseTool(breaking_dl); }
    else sc.DeleteACSChartDrawing(sc.ChartNumber, TOOL_DELETE_CHARTDRAWING, 11);
    if (sc.Close[sc.IndexOfLastVisibleBar] > mid[sc.IndexOfLastVisibleBar]) sc.DeleteACSChartDrawing(sc.ChartNumber, TOOL_DELETE_CHARTDRAWING, 11);

    if (display_values.GetYesNo())//for debugging only.. after it is done this will be deleted..
    {
        {
            s_UseTool t;
            t.Clear();
            t.ChartNumber = sc.ChartNumber;
            t.DrawingType = DRAWING_TEXT;
            t.FontSize = 10;
            t.FontBold = false;
            t.AddMethod = UTAM_ADD_OR_ADJUST;
            t.UseRelativeVerticalValues = 1;
            t.Color = RGB(255, 255, 255);
            t.Region = 0;
            t.Text.Format("Time of bar that made last daily high at price %4.2f %i-%i-%i  %i:%i:%0.2i", perzist_daily_high, dh_year, dh_month, dh_day, dh_hour, dh_minute, dh_second);
            t.LineNumber = 2;
            t.BeginDateTime = 5;
            t.BeginValue = 94;
            sc.UseTool(t);
        } {
            s_UseTool t;
            t.Clear();
            t.ChartNumber = sc.ChartNumber;
            t.DrawingType = DRAWING_TEXT;
            t.FontSize = 10;
            t.FontBold = false;
            t.AddMethod = UTAM_ADD_OR_ADJUST;
            t.UseRelativeVerticalValues = 1;
            t.Color = RGB(255, 255, 255);
            t.Region = 0;
            t.Text.Format("Time of bar that made last daily low at price %4.2f is %i-%i-%i %i:%0.2i:%0.2i", perzist_daily_low, dl_year, dl_month, dl_day, dl_hour, dl_minute, dl_second);
            t.LineNumber = 3;
            t.BeginDateTime = 5;
            t.BeginValue = 92;
            sc.UseTool(t);
        } {
            s_UseTool t;
            t.Clear();
            t.ChartNumber = sc.ChartNumber;
            t.DrawingType = DRAWING_TEXT;
            t.FontSize = 10;
            t.FontBold = false;
            t.AddMethod = UTAM_ADD_OR_ADJUST;
            t.UseRelativeVerticalValues = 1;
            t.Color = RGB(255, 255, 255);
            t.Region = 0;
            t.Text.Format("Time of current bar: %i-%i-%i %i:%0.2i:%0.2i", year, month, day, hour, minute, second);
            t.LineNumber = 4;
            t.BeginDateTime = 5;
            t.BeginValue = 90;
            sc.UseTool(t);
        } {
            s_UseTool t;
            t.Clear();
            t.ChartNumber = sc.ChartNumber;
            t.DrawingType = DRAWING_TEXT;
            t.FontSize = 10;
            t.FontBold = false;
            t.AddMethod = UTAM_ADD_OR_ADJUST;
            t.UseRelativeVerticalValues = 1;
            t.Color = RGB(255, 255, 255);
            t.Region = 0;
            t.Text.Format("Time of bar %i back: %i-%i-%i %i:%0.2i:%0.2i", nr_of_mins_back.GetInt(), pb_year, pb_month, pb_day, pb_hour, pb_minute, pb_second);
            t.LineNumber = 5;
            t.BeginDateTime = 5;
            t.BeginValue = 88;
            sc.UseTool(t);
        }
    }
}
