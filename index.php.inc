<?php

	ini_set('display_errors',false);

	/**
	 * ==========================================================
	 * 						ERROR HANDLING
	 * ==========================================================
	 */
	function global_error_handler($error_level,$error_message,$error_file,$error_line,$error_context) {
	
		if(!file_exists('errors.log')) file_put_contents('errors.log','');
		
		$log_message='['.date(DATE_RFC2822).']:';
		
		if($error_level==1) $log_message.='[PHP_ERROR]';
		else if($error_level==2) $log_message.='[PHP_WARNING]';
		else if($error_level==4) $log_message.='[PHP_PARSE]';
		else if($error_level==8) $log_message.='[PHP_NOTICE]';
		else if($error_level==256) $log_message.='[APP_ERROR]';
		else if($error_level==512) $log_message.='[APP_WARNING]';
		else if($error_level==1024) $log_message.='[APP_NOTICE]';
		
		$log_message.=sprintf(' "%s" (on file %s at line %s)',$error_message,$error_file,$error_line);
		
		error_log($log_message."\n",3,'errors.log');
		
		die('An error occurred. Could not continue.');
	}
	
	set_error_handler('global_error_handler');
	
	/**
	 * ==========================================================
	 * 						AUTOLOAD METHODS
	 * ==========================================================
	 */
	
	function load_module($class_name) {
	
		$file_name=PATH_MODULES.$class_name.'.php';
		
		if(is_readable($file_name)) {
			require_once($file_name);
			if(method_exists($class_name,'init')) $class_name::init();
		}
	}
		
	function load_redbean($class_name) {
		
		if($class_name !== 'R') return;
		
		require_once('vendor/redbeanphp-3.5.4/rb.php');
		
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
	
	spl_autoload_register('load_module');
	spl_autoload_register('load_redbean');
	spl_autoload_register('load_swiftmailer');
	
	//require_once('vendor/autoload.php');
	
	/**
	 * ==========================================================
	 * 					COMMON UTILITY METHODS
	 * ==========================================================
	 */
	 
	function getURIParameters() {
		
		$uriResource=explode('?',$_SERVER['REQUEST_URI']);
		$scriptResource=explode('?',$_SERVER['SCRIPT_NAME']);
		
		$requestURI=explode('/',$uriResource[0]);
		$scriptName=explode('/',$scriptResource[0]);

		for($i=0;$i<sizeof($scriptName);$i++) { if($requestURI[$i]==$scriptName[$i]) unset($requestURI[$i]); }
		
		$requestURI=array_filter($requestURI);
		$requestParams=array_values($requestURI);
		
		//$requestParams=array_merge($requestParams,$_REQUEST);
		
		return $requestParams;
	}
	
	function shutdown_system() {
		
		if(class_exists('DataStore',false)) DataStore::disconnectFromDatabase();
		
	}
	
	function generate_random_string($length = 25) {
		
		if($length < 5) $length = 5;
		
		$pool = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
		$chunk_size = 5;
		
		$num_chunks = intval($length / $chunk_size);
		$extra = $length % $chunk_size;
		
		$random_string = '';
		
		for($index = 0; $index < $num_chunks; $index++) {
			$chunk = substr(str_shuffle($pool), 0, $chunk_size);
			$random_string = $random_string.$chunk;
		}
		
		$chunk = substr(str_shuffle($pool), 0, $extra);
		$random_string = $random_string.$chunk;
		
		return $random_string;
	}
	
	/**
	 * ==========================================================
	 * 							RESOLVER
	 * ==========================================================
	 */
	 
	function resolve($action,$uriParams) {
		
		if($action) {
			
			$portConfig = Registry::lookupPort($action);
			
			if(!$portConfig) {
				if($action === 'default') die("Your app does not have a default port configuration. Please configure the default port.");
				else die("Your request cannot be resolved.");
			}
			
			if($portConfig[2] === Registry::PORT_TYPE_PRIVATE && !Session::isRunning()) {
				header('Location: '.BASE_URI);
				return;
			}
			
			$controllerName=$portConfig[0].'Controller';
			$methodName=$portConfig[1];
			
			$controller=new $controllerName();
			
			call_method($controller, $methodName, $uriParams);
		} else die('Your request cannot be resolved.');
		
	}
	
	function call_method($object, $method_name, $args) {
		
		$num_args = count($args);
		
		if($num_args == 0) {
			return $object->{$method_name}();
		} else if($num_args == 1) {
			return $object->{$method_name}($args[0]);
		} else if($num_args == 2) {
			return $object->{$method_name}($args[0], $args[1]);
		} else if($num_args == 3) {
			return $object->{$method_name}($args[0], $args[1], $args[2]);
		} else {
			return call_user_func_array(array($object, $method_name), $args);
		}
	}
	
	register_shutdown_function('shutdown_system');
	
	$uriParams=getURIParameters();
	
	/**
	 * Handle static content separately.
	 */
	if(count($uriParams)>0) {
		$info=pathinfo($uriParams[count($uriParams)-1]);
		if(isset($info['extension'])) {
			$info['extension']=strtolower($info['extension']);
			if(in_array($info['extension'],array('js','css','jpg','jpeg','png','gif','bmp','svg','eot','ttf','woff','otf'))) require_once('core/static.php');
		}
	}
	
	/**
	 * Explicitly set command for default view.
	 */
	if(count($uriParams)==0) {
		$action='default';
	} else {
		$action=$uriParams[0];
		$uriParams=array_slice($uriParams,1);
	}
	
	/**
	 * Resolve URI request.
	 */
	resolve($action,$uriParams);

?>
