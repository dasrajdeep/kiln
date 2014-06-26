<?php
	
	defined('SYSTEM_STARTED') or die('You are not permitted to access this resource.');
	
	if(!class_exists('ContentManager')) require_once('core/bootstrap.php');
	
	ContentManager::serveContent($info['basename']);
	
	die();

?>
