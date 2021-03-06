<?php
// $Id$

/** mrbsCheckFree()
 * 
 * Check to see if the time period specified is free
 * 
 * $room_id   - Which room are we checking
 * $starttime - The start of period
 * $endtime   - The end of the period
 * $ignore    - An entry ID to ignore, 0 to ignore no entries
 * $repignore - A repeat ID to ignore everything in the series, 0 to ignore no series
 * 
 * Returns:
 *   nothing   - The area is free
 *   something - An error occured, the return value is human readable
 */
function mrbsCheckFree($room_id, $starttime, $endtime, $ignore, $repignore)
{
	# Select any meetings which overlap ($starttime,$endtime) for this room:
	$sql = "SELECT id, name FROM mrbs_entry WHERE
		start_time < $endtime AND end_time > $starttime
		AND room_id = $room_id";

	if ($ignore > 0)
		$sql .= " AND id <> $ignore";
	if ($repignore > 0)
		$sql .= " AND repeat_id <> $repignore";

	$res = sql_query($sql);
	
	if(! $res)
		return sql_error();
	
	$err = "";

	for ($i = 0; ($row = sql_row($res, $i)); $i++)
		$err .= "<LI><A HREF=\"view_entry.php?id=$row[0]\">$row[1]</A>";
	
	return $err;
}

/** mrbsDelEntry()
 * 
 * Delete an entry, or optionally all entrys.
 * 
 * $user   - Who's making the request
 * $id     - The entry to delete
 * $series - If set, delete the series, except user modified entrys
 * $all    - If set, include user modified entrys in the series delete
 *
 * Returns:
 *   0        - An error occured
 *   non-zero - The entry was deleted
 */
function mrbsDelEntry($user, $id, $series, $all)
{
	$repeat_id = sql_query1("SELECT repeat_id FROM mrbs_entry WHERE id=$id");
	if ($repeat_id < 0)
		return 0;
	
	$sql = "SELECT create_by, id, entry_type FROM mrbs_entry WHERE ";
	
	if($series)
		$sql .= "repeat_id=$repeat_id";
	else
		$sql .= "id=$id";
	
	$res = sql_query($sql);
	
	$removed = 0;
	
	for ($i = 0; ($row = sql_row($res, $i)); $i++)
	{
		if(!getWritable($row[0], $user))
			continue;
		
		if($series && $row[2] == 2 && !$all)
			continue;
		
		if (sql_command("DELETE FROM mrbs_entry WHERE id=" . $row[1]) > 0)
			$removed++;
	}
	
	if ($repeat_id > 0 &&
            sql_query1("SELECT count(*) FROM mrbs_entry WHERE repeat_id=$repeat_id") == 0)
		sql_command("DELETE FROM mrbs_repeat WHERE id=$repeat_id");
	
	return $removed > 0;
}

/** mrbsCreateSingleEntry()
 * 
 * Create a single (non-repeating) entry in the database
 * 
 * $starttime   - Start time of entry
 * $endtime     - End time of entry
 * $entry_type  - Entry type
 * $repeat_id   - Repeat ID
 * $room_id     - Room ID
 * $owner       - Owner
 * $name        - Name
 * $type        - Type (Internal/External)
 * $description - Description
 * 
 * Returns:
 *   0        - An error occured while inserting the entry
 *   non-zero - The entry's ID
 */
function mrbsCreateSingleEntry($starttime, $endtime, $entry_type, $repeat_id, $room_id,
                               $owner, $name, $type, $description)
{
	$name        = slashes($name);
	$description = slashes($description);
	
	$sql = "INSERT INTO mrbs_entry (  start_time,   end_time,   entry_type,    repeat_id,   room_id,
	                                  create_by,    name,       type,          description)
	                        VALUES ($starttime, $endtime, $entry_type, $repeat_id, $room_id,
	                                '$owner',     '$name',    '$type',       '$description')";
	
	if (sql_command($sql) < 0) return 0;
	
	return sql_insert_id("mrbs_entry", "id");
}

/** mrbsCreateRepeatEntry()
 * 
 * Creates a repeat entry in the data base
 * 
 * $starttime   - Start time of entry
 * $endtime     - End time of entry
 * $rep_type    - The repeat type
 * $rep_enddate - When the repeating ends
 * $rep_opt     - Any options associated with the entry
 * $room_id     - Room ID
 * $owner       - Owner
 * $name        - Name
 * $type        - Type (Internal/External)
 * $description - Description
 * 
 * Returns:
 *   0        - An error occured while inserting the entry
 *   non-zero - The entry's ID
 */
function mrbsCreateRepeatEntry($starttime, $endtime, $rep_type, $rep_enddate, $rep_opt,
                               $room_id, $owner, $name, $type, $description)
{
	$name        = slashes($name);
	$description = slashes($description);
	
	$sql = "INSERT INTO mrbs_repeat (  start_time,   end_time,    rep_type,    end_date,       rep_opt,
	                                   room_id,      create_by,   type,        name,           description)
	                         VALUES ($starttime, $endtime,  $rep_type, $rep_enddate, '$rep_opt',
	                                 $room_id,   '$owner',    '$type',     '$name',        '$description')";
	
	if (sql_command($sql) < 0) return 0;
	
	return sql_insert_id("mrbs_repeat", "id");
}


/** same_day_next_month
 *  Return the number of days to step forward for a "monthly repeat,
 *  corresponding day" series - same week number and day of week next month.
 *  This function always returns either 28 or 35.
 *  For dates after the 28th day of a month, the results are undefined.
 */
function same_day_next_month($time)
{
	$days_in_month = date("t", $time);
	$day = date("d", $time);
	$weeknumber = (int)(($day - 1) / 7) + 1;
	if ($day + 7 * (5 - $weeknumber) <= $days_in_month) return 35;
	else return 28;
}

/** mrbsGetRepeatEntryList
 * 
 * Returns a list of the repeating entrys
 * 
 * $time     - The start time
 * $enddate  - When the repeat ends
 * $rep_type - What type of repeat is it
 * $rep_opt  - The repeat entrys
 * $max_ittr - After going through this many entrys assume an error has occured
 * 
 * Returns:
 *   empty     - The entry does not repeat
 *   an array  - This is a list of start times of each of the repeat entrys
 */
function mrbsGetRepeatEntryList($time, $enddate, $rep_type, $rep_opt, $max_ittr)
{
	$sec   = date("s", $time);
	$min   = date("i", $time);
	$hour  = date("G", $time);
	$day   = date("d", $time);
	$month = date("m", $time);
	$year  = date("Y", $time);
	
	$entrys = "";
	for($i = 0; $i < $max_ittr; $i++)
	{
		$time = mktime($hour, $min, $sec, $month, $day, $year);
		if ($time > $enddate)
			break;

		$entrys[$i] = $time;
		
		switch($rep_type)
		{
			// Daily repeat
			case 1:
				$day += 1;
				break;
			
			// Weekly repeat
			case 2:
				$j = $cur_day = date("w", $entrys[$i]);
				// Skip over days of the week which are not enabled:
				while (($j = ($j + 1) % 7) != $cur_day && !$rep_opt[$j])
					$day += 1;

				$day += 1;
				break;
			
			// Monthly repeat
			case 3:
				$month += 1;
				break;
			
			// Yearly repeat
			case 4:
				$year += 1;
				break;
			
			// Monthly repeat on same week number and day of week
			case 5:
				$day += same_day_next_month($time);
				break;
				
			// Unknown repeat option
			default:
				return;
		}
	}
	
	return $entrys;
}

/** mrbsCreateRepeatingEntrys()
 * 
 * Creates a repeat entry in the data base + all the repeating entrys
 * 
 * $starttime   - Start time of entry
 * $endtime     - End time of entry
 * $rep_type    - The repeat type
 * $rep_enddate - When the repeating ends
 * $rep_opt     - Any options associated with the entry
 * $room_id     - Room ID
 * $owner       - Owner
 * $name        - Name
 * $type        - Type (Internal/External)
 * $description - Description
 * 
 * Returns:
 *   0        - An error occured while inserting the entry
 *   non-zero - The entry's ID
 */
function mrbsCreateRepeatingEntrys($starttime, $endtime, $rep_type, $rep_enddate, $rep_opt,
                                   $room_id, $owner, $name, $type, $description)
{
	global $max_rep_entrys;
	
	$reps = mrbsGetRepeatEntryList($starttime, $rep_enddate, $rep_type, $rep_opt, $max_rep_entrys);
	if(count($reps) > $max_rep_entrys)
		return 0;
	
	if(empty($reps))
	{
		mrbsCreateSingleEntry($starttime, $endtime, 0, 0, $room_id, $owner, $name, $type, $description);
		return;
	}
	
	$ent = mrbsCreateRepeatEntry($starttime, $endtime, $rep_type, $rep_enddate, $rep_opt, $room_id, $owner, $name, $type, $description);
	if($ent)
	{
		$diff = $endtime - $starttime;
	
		for($i = 0; $i < count($reps); $i++)
			mrbsCreateSingleEntry($reps[$i], $reps[$i] + $diff, 1, $ent,
				 $room_id, $owner, $name, $type, $description);
	}
	return $ent;
}

/* mrbsGetEntryInfo()
 *
 * Get the booking's entrys
 * 
 * $id = The ID for which to get the info for.
 * 
 * Returns:
 *    nothing = The ID does not exist
 *    array   = The bookings info
 */
function mrbsGetEntryInfo($id)
{
	$sql = "SELECT start_time, end_time, entry_type, repeat_id, room_id,
	               timestamp, create_by, name, type, description
                FROM mrbs_entry WHERE (ID = $id)";
	
	$res = sql_query($sql);
	if (! $res) return;
	
	$ret = "";
	if(sql_count($res) > 0)
	{
		$row = sql_row($res, 0);
		
		$ret["start_time"]  = $row[0];
		$ret["end_time"]    = $row[1];
		$ret["entry_type"]  = $row[2];
		$ret["repeat_id"]   = $row[3];
		$ret["room_id"]     = $row[4];
		$ret["timestamp"]   = $row[5];
		$ret["create_by"]   = $row[6];
		$ret["name"]        = $row[7];
		$ret["type"]        = $row[8];
		$ret["description"] = $row[9];
	}
	sql_free($res);
	
	return $ret;
}

function mrbsGetRoomArea($id)
{
	$id = sql_query1("SELECT area_id FROM mrbs_room WHERE (id = $id)");
	if ($id <= 0) return 0;
	return $id;
}

?>
