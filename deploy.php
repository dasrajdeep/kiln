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

require_once('app/environment.php');
require_once('core/bootstrap.php');

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