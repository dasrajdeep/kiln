<?php

defined('SYSTEM_STARTED') or die('You are not permitted to access this resource.');

function load_module($class_name) {
	
	$file_name=PATH_MODULES.$class_name.'.php';
	
	if(is_readable($file_name)) {
		require_once($file_name);
		if(method_exists($class_name,'init')) $class_name::init();
	}
}

function load_system($class_name) {
	
	$file_name=PATH_SYSTEM.$class_name.'.php';
	
	if(is_readable($file_name)) {
		require_once($file_name);
		if(method_exists($class_name,'init')) $class_name::init();
	}
}

function load_controller($class_name) {
	
	$file_name=PATH_CONTROLLERS.$class_name.'.php';
	
	if(is_readable($file_name)) {
		require_once($file_name);
		if(method_exists($class_name,'init')) $class_name::init();
	}
}

function load_redbean($class_name) {
	
	if($class_name !== 'R') return;
	
	require_once('vendor/_required/redbeanphp-3.5.4/rb.php');
	
	$db_host=Registry::lookupConfig(Registry::CONFIG_TYPE_DATABASE, 'host');
	$db_user=Registry::lookupConfig(Registry::CONFIG_TYPE_DATABASE, 'username');
	$db_pass=Registry::lookupConfig(Registry::CONFIG_TYPE_DATABASE, 'password');
	$db_name=Registry::lookupConfig(Registry::CONFIG_TYPE_DATABASE, 'name');
	
	R::setup(sprintf('mysql:host=%s;dbname=%s',$db_host,$db_name),$db_user,$db_pass);
}

function load_swiftmailer($class_name) {
	
	if(strpos($class_name, 'Swift') != 0) return;
	
	require_once('vendor/_required/swift-5.1.0/lib/swift_required.php');
}

?>
