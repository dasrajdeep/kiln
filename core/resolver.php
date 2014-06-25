<?php
	
	defined('SYSTEM_STARTED') or die('You are not permitted to access this resource.');
	
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

?>
