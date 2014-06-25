<?php
	
	/**
	 * --------------------------------
	 * APPLICATION ENVIRONMENT SETTINGS
	 * --------------------------------
	 */
	
	defined('SYSTEM_STARTED') or die('You are not permitted to access this resource.');
	
	/**
	 * Set the locale to a standard locale string.
	 * 
	 * List of valid locales can be found at:
	 * http://www.loc.gov/standards/iso639-2/php/code_list.php
	 * 
	 * Default is 0 (current locale).
	 */
	$env_locale					 = '0';
	
	/**
	 * Set the timezone to a standard timezone string.
	 * 
	 * List of valid timezones can be found at:
	 * http://www.php.net/manual/en/timezones.php
	 */
	$env_time_zone				 = 'Asia/Kolkata';
	
	/**
	 * Set the base URI of the app.
	 * 
	 * By default this is set to /. 
	 * However, if your app resides in a sub-directory of 
	 * your domain, then you must change this value to the 
	 * appropriate name of the sub-directory.
	 */
	$env_base_uri 				 = '/kiln/';

?>
