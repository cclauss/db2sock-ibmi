<?php
$url = getenv("SQL_HOST400");                                                                 
$db = getenv("SQL_DB400");                                                                 
$uid = getenv("SQL_UID400");                                                                 
$pwd = getenv("SQL_PWD400");
if (!$url) {
  $url = "http://ut28p63/db2/db2json.pgm";
}
?>