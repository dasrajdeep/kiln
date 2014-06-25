<?php
	
	/**
	 * ----------------------------------
	 * GLOBAL CONSTANTS ARE DEFINED HERE.
	 * ----------------------------------
	 */
	
	defined('SYSTEM_STARTED') or die('You are not permitted to access this resource.');
	
	define('DS',DIRECTORY_SEPARATOR);
	
	define('BASE_DIR',getcwd().'/');
	define('BASE_URI',$env_base_uri);	
	
	define('PATH_CACHE','cache/');
	define('PATH_SYSTEM','system/');
	define('PATH_MODULES','app/modules/');
	define('PATH_HELPERS','app/helpers/');
	define('PATH_CONTROLLERS','app/controllers/');
	define('PATH_THIRD_PARTY','app/external/');
	define('PATH_VIEWS','app/views/');
	define('PATH_SCRIPTS','app/scripts/');
	define('PATH_STYLES','app/styles/');
	define('PATH_GRAPHICS','app/images/');
	define('PATH_FONTS','app/fonts/');
	define('PATH_APPDATA','data/');
	
?>
