<?php
	
/**
 * =====================================================
 * 						BOOTSTRAP
 * =====================================================
 */
if(php_sapi_name() !== 'cli') {
	die('YOU ARE NOT ALLOWED TO ACCESS THIS RESOURCE');
}

define('SYSTEM_STARTED', TRUE);

if(!file_exists('.docroot') && !file_exists('index.php')) {
	copy('index.php.dev', 'index.php');
	die("Please run the app from a browser first.\n");
}

require_once('core/errorhandler.php');
	
set_error_handler('global_error_handler');
	
require_once('core/globals.php');
require_once('core/autoload.php');
require_once('core/common.php');
require_once('core/resolver.php');
require_once('vendor/autoload.php');

spl_autoload_register('load_system');
spl_autoload_register('load_module');
spl_autoload_register('load_controller');
spl_autoload_register('load_redbean');
spl_autoload_register('load_swiftmailer');

register_shutdown_function('shutdown_system');

/**
 * =====================================================
 */

if(count($argv) < 2) {
	show_instructions();
} else {
	$command = $argv[1];
	
	if($command === 'production') {
		if(file_exists('production')) {
			echo shell_exec("rm -rf production");
		} 
		
		mkdir('production');
		echo shell_exec("chmod -R 777 production");
		 
		Optimizer::optimize();
		
		if(file_exists('index.php')) unlink('index.php');
		copy('index.php.live', 'index.php');
	} else if($command === 'dev') {
		if(file_exists('production')) {
			echo shell_exec("rm -rf production");
		}
		
		if(file_exists('index.php')) unlink('index.php');
		copy('index.php.dev', 'index.php');
	} else {
		show_instructions();
	}
}

function show_instructions() {
	echo "Usage: php deploy.php [production/dev]\n";
} 

?>