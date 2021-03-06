<?php

include "config.inc";
include "functions.inc";
include "connect.inc";
include "mrbs_auth.inc";

function add_duration ( $time, $duration ) {
  $list = split ( ":", $time );
  $hour = $list[0];
  $min = $list[1];
  $minutes = $hour * 60 + $min + $duration;
  $h = $minutes / 60;
  $m = $minutes % 60;
  $ret = sprintf ( "%d:%02d", $h, $m );
  //echo "add_duration ( $time, $duration ) = $ret <BR>";
  return $ret;
}

// check to see if two events overlap
function times_overlap ( $time1, $duration1, $time2, $duration2 ) {
  //echo "times_overlap ( $time1, $duration1, $time2, $duration2 )<BR>";
  $list1 = split ( ":", $time1 );
  $hour1 = $list1[0];
  $min1 = $list1[1];
  $list2 = split ( ":", $time2 );
  $hour2 = $list2[0];
  $min2 = $list2[1];
  // convert to minutes since midnight
  $tmins1start = ($hour1 * 60 + $min1) * 60;
  $tmins1end = $tmins1start + ($duration1 * 60) - 1;
  $tmins2start = ($hour2 * 60 + $min2) * 60;
  $tmins2end = $tmins2start + ($duration2 * 60) - 1;
  //echo "tmins1start=$tmins1start, tmins1end=$tmins1end, tmins2start=$tmins2start, tmins2end=$tmins2end<BR>";
  if ( $tmins1start >= $tmins2start && $tmins1start <= $tmins2end )
    return true;
  if ( $tmins1end >= $tmins2start && $tmins1end <= $tmins2end )
    return true;
  if ( $tmins2start >= $tmins1start && $tmins2start <= $tmins1end )
    return true;
  if ( $tmins2end >= $tmins1start && $tmins2end <= $tmins1end )
    return true;
  return false;
}

// Units start in seconds
$units = 1.0;

switch($dur_units)
{
	case "years":
		$units *= 52;
	case "weeks":
		$units *= 7;
	case "days":
		$units *= 24;
	case "hours":
		$units *= 60;
	case "minutes":
		$units *= 60;
	case "seconds":
		break;
}

// Units are now in "$dur_units" numbers of seconds

$starttime = mktime($hour, $minute, 0, $month, $day, $year);
$endtime   = mktime($hour, $minute, 0, $month, $day, $year) + ($units * $duration);

if($all_day == "yes")
	$round_up = 60 * 60 * 24;
else
	$round_up = 30 * 60;

$diff = $endtime - $starttime;

if($tmp = $diff % $round_up)
	$endtime += $round_up - $tmp;

if($all_day == "yes")
{
	$diff = $endtime - $starttime;
	
	if($tmp = $starttime % (60 * 60 * 24))
	{
		$starttime -= $tmp;
		$endtime    = $starttime + $diff;
	}	
}

$endtime1  = $endtime - 1;

# first check for any schedule conflicts
# we ask the db if there is anything which
#   starts before this and ends after the start
#   or starts between the times this starts and ends
#   where the room is the same

$sql = "select id, name from mrbs_entry where 
(
  (start_time between '$starttime' and $endtime1)
  or
  ('$starttime' between start_time and date_sub(end_time, interval 1 second))
)
and room_id = $room_id
";
# if this is a replacement then dont conflict with itself
if ($id) {$sql = "$sql and id <> $id";}


$res = mysql_query($sql);
echo mysql_error();

# Make sure we remember which appointments overlap the one were trying to add
if (mysql_num_rows($res) > 0) {
	$error = $lang[conflict];
	while ($row = mysql_fetch_row($res)) {
		$error = "$error<br><a href=view_entry.php3?id=$row[0]>$row[1]</a>";
	}
}


if (strlen($error) == 0) {
	# now add the entries
	if ($id) {
		# This is to replace an existing entry
		mysql_query("delete from mrbs_entry where id=$id");
	}
	#actually do some adding
	$name_q        = addslashes($name);
	$description_q = addslashes($description);
	$sql = "insert into mrbs_entry (room_id, create_by, start_time, end_time, type, name, description) values (
	        '$room_id',
			  '".getUserName()."',
			  '$starttime',
			  '$endtime',
			  '$type',
			  '$name_q',
			  '$description_q'
			  )";
	
#	echo "$sql<p>";
	mysql_query($sql);
	echo mysql_error();
	# Now its all done go back to the day view
	if (strlen($returl) == 0) {
		$returl = "day.php3?year=$year&month=$month";
	}
	Header ( "Location: $returl" );
	exit;
}

?>
<HTML>
<HEAD><TITLE><?echo $lang[mrbs]?></TITLE>
<?include "style.inc"?>
</HEAD>
<BODY>

<?php if ( strlen ( $overlap ) ) { ?>
<H2><FONT COLOR="<?php echo $H2COLOR;?>">Scheduling Conflict</H2></FONT>

Your suggested time of <B>
<?php
  $time = sprintf ( "%d:%02d", $hour, $minute );
  echo display_time ( $time );
  if ( $duration > 0 )
    echo "-" . display_time ( add_duration ( $time, $duration ) );
?>
</B> conflicts with the following existing calendar entries:
<UL>
<?php echo $overlap; ?>
</UL>

<?php } else { ?>
<H2><FONT COLOR="<?php echo $H2COLOR;?>"><?echo $lang[error]?></H2></FONT>
<BLOCKQUOTE>
<?php echo $error; ?>
</BLOCKQUOTE>

<?php } 

echo "<a href=$returl>$lang[returncal]</a><p>";

include "trailer.inc"; ?>

</BODY>
</HTML>
