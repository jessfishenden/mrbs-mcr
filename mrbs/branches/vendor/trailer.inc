
<BR CLEAR="all">
<HR CLEAR="all">
<B>View Day:</B>
<?php

if ($year && $month && $day) {
	$btime = mktime(0, 0, 0, $month, $day, $year);
	$thisyear=$year; $thismonth=$month; $thisday=$day;
} else {
	$btime = time();
}

$stime = $btime - (7*24*60*60);
$etime = $btime + (7*24*60*60);

for ($i = $stime; $i <= $etime; $i=$i+(24*60*60)) {
	$cyear  = date("Y",$i);
	$cmonth = date("m",$i);
	$ddate  = date("M d",$i);
	$cday   = date("d",$i);
	echo "<a href=day.php3?year=$cyear&month=$cmonth&day=$cday>$ddate</a>, ";
}

?>
<br>
<BR>

<hr>
<p>


