<?php

# $Id$

###########################################################################
#   MRBS Configuration File
#   Configure this file for your site.
#   You shouldn't have to modify anything outside this file.
###########################################################################

###################
# Database settings
###################
# Which database system: "pgsql"=PostgreSQL, "mysql"=MySQL
$dbsys = "mysql";
# Hostname of database server. For pgsql, can use "" instead of localhost
# to use Unix Domain Sockets instead of TCP/IP.
$db_host = "localhost";
# Database name:
$db_database = "mrbs";
# Database login user name:
$db_login = "mrbs";
# Database login password:
$db_password = "mrbs-password";
# Uncomment this to NOT use PHP persistent (pooled) database connections:
# $db_nopersist = 1;

################################
# Site identification information
#################################
$mrbs_admin = "Your Administrator";
$mrbs_admin_email = "admin_email@your.org";
$mrbs_company = "Your Company";

###################
# Calendar settings
###################
# Resolution - what blocks can be booked, in seconds.
# Default is half an hour: 1800 seconds.
$resolution = 1800;

# Start and end of day, NOTE: These are integer hours only, 0-23, and
# morningstarts must be < eveningends. See also eveningends_minutes.
$morningstarts = 7;
$eveningends   = 19;

# Minutes to add to $eveningends hours to get the real end of the day.
# Examples: To get the last slot on the calendar to be 16:30-17:00, set
# eveningends=16 and eveningends_minutes=30. To get a full 24 hour display
# with 15-minute steps, set morningstarts=0; eveningends=23;
# eveningends_minutes=45; and resolution=900.
$eveningends_minutes = 0;

# Start of week: 0 for Sunday, 1 for Monday, etc.
$weekstarts = 0;

# Trailer date format: 0 to show dates as "Jul 10", 1 for "10 Jul" 
$dateformat = 0;

########################
# Miscellaneous settings
########################

# Maximum repeating entrys (max needed +1):
$max_rep_entrys = 365 + 1;

# Default report span in days:
$default_report_days = 60;

# Results per page for searching:
$search["count"] = 20;

###############################################
# Authentication settings - read AUTHENTICATION
###############################################
# IP authentication allows any user to create bookings.
$auth["realm"]  = "";
$auth["type"]   = "ip";
$auth["prog"]   = "";
$auth["params"] = "";

# The various level two users (can modify other peoples settings)
# By default, only localhost is an administrator.
$auth["admin"][] = "127.0.0.1";
#$auth["admin"][] = "10.0.0.1";
#$auth["admin"][] = "10.0.0.2";
#$auth["admin"][] = "10.0.0.3";

# 'auth_ldap' configuration settings
# Where is the LDAP server
#$ldap_host = "localhost";
# LDAP base distinguish names
#$ldap_base_dn[] = "ou=organizationalunit1,o=organization,c=MY";
#$ldap_base_dn[] = "ou=organizationalunit2,o=organization,c=MY";
#$ldap_base_dn[] = "ou=organizationalunit3,o=organization,c=MY";

##########
# Language
##########

# Change the en below to the code for your language - if 
# there is a language file for it.
include 'lang.en';

# These two lines import a language based on what the client
# is using. Comment them out to disable this.
$lang_file = "lang.$HTTP_ACCEPT_LANGUAGE";
if (file_exists($lang_file)) {include $lang_file;}

# We also want to do locales for Dates/Times etc.
# Use the setlocale() function for this.

# Define the default locale here. For a list of supported
# locales on your system do "locale -a"
setlocale('LC_ALL','C');

# We attempt to make up a sensible locale from the HTTP_ACCEPT_LANGUAGE
# environment variable. If this doesn't work for you, comment it out
# and assign locale directly.
# If HTTP_ACCEPT_LANGUAGE is a comma-separated list, take the first one.
$locale = ereg_replace(",.*", "", $HTTP_ACCEPT_LANGUAGE);
#
# Convert locale=xx to xx_XX; this is not correct for some locales???
if (strlen($locale) == 2) {
   $locale = strtolower($locale)."_".strtoupper($locale);
} else {
   $locale = strtolower(substr($locale,0,2))."_".strtoupper(substr($locale,3,2));
}
setlocale('LC_ALL',$locale);

#############
# Entry Types
#############
# This array maps entry type codes (letters A through J) into descriptions.
# Each type has a color (see TD.x classes in the style sheet mrbs.css).
#    A=Pink  B=Blue-green  C=Peach  D=Yellow      E=Light blue
#    F=Tan   G=Red         H=Aqua   I=Light green J=Gray
# The value for each type is a short (one word is best) description of the
# type. The values must be escaped for HTML output ("R&amp;D").
# Please leave I and E alone for compatibility.
# If a type's entry is unset or empty, that type is not defined; it will not
# be shown in the day view color-key, and not offered in the type selector
# for new or edited entries.

# $typel["A"] = "A";
# $typel["B"] = "B";
# $typel["C"] = "C";
# $typel["D"] = "D";
$typel["E"] = $lang["external"];
# $typel["F"] = "F";
# $typel["G"] = "G";
# $typel["H"] = "H";
$typel["I"] = $lang["internal"];
# $typel["J"] = "J";

##########################################
# PHP System Configuration - do not change
##########################################
# Disable magic quoting on database returns:
set_magic_quotes_runtime(0);

?>
