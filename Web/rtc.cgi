t <html><head><title>RTC</title>
t <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t <script language=JavaScript type="text/javascript">
# Define URL and refresh timeout
t var formUpdate = new periodicObj("rtc.cgx", 500);
t function periodicUpdateRtc() {
t  if(document.getElementById("rtcChkBox").checked == true) {
t   updateMultiple(formUpdate);
t   rtc_elTime = setTimeout(periodicUpdateRtc, formUpdate.period);
t  }
t  else
t   clearTimeout(rtc_elTime);
t }
t </script></head>
i pg_header.inc
t <h2 align="center"><br>Time of the RTC</h2>
t <p><font size="2">This page allows you to monitor time in the system RTC . 
t  Periodic screen update is based on <b>xml</b> technology.</font></p>
t <form action="rtc.cgi" method="post" name="rtc">
t <input type="hidden" value="rtc" name="rtc">
t <table border=0 width=99%><font size="3">
t <tr style="background-color: #00ffff">
t  <th width=15%>Item</th>
t  <th width=15%>Value</th></tr>
t <tr><td>Hora</td>
t   <td align="center">
t <input type="text" readonly style="background-color: transparent; border: 0px"
c h 1  size="10" id="time" value="%s"></td></tr>
t <tr><td>Fecha</td>
t   <td align="center">
t <input type="text" readonly style="background-color: transparent; border: 0px"
c h 2  size="10" id="date" value="%s"></td></tr>
t </font></table>
t <p align=center>
t <input type=button value="Refresh" onclick="updateMultiple(formUpdate)">
t Periodic:<input type="checkbox" id="rtcChkBox" onclick="periodicUpdateRtc()">
t </p></form>
i pg_footer.inc
. End of script must be closed with period
