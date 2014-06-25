<?php

defined('SYSTEM_STARTED') or die('You are not permitted to access this resource.');

class ViewManager {
	
	private static $locations = array(
		'js'=>PATH_SCRIPTS,
		'css'=>PATH_STYLES,
		'eot'=>PATH_FONTS,
		'ttf'=>PATH_FONTS,
		'woff'=>PATH_FONTS,
		'svg'=>PATH_FONTS,
		'otf'=>PATH_FONTS
	);
	
	public static function renderView($viewName, $var=null) {
		
		if(PRODUCTION) {
			require_once('production/'.$viewName.'.inc');
			return;
		}
		
		$GLOBALS['view_type']='partial';
		
		$view_registry=parse_ini_file(PATH_VIEWS.'.views',true);
		$view_registry=$view_registry['view_registry'];
		
		if(isset($view_registry[$viewName])) {
			ob_start();
			require_once($view_registry[$viewName]);
			$html_body=ob_get_clean();
			
			if(!isset($GLOBALS['view_config'])) $GLOBALS['view_config']=array('dependancies'=>array());
			
			if($GLOBALS['view_type']==='complete') require_once('core/container.php');
			else echo $html_body;
			//Optimizer::refactorFlows();
			return true;
		} else return false;
	}
	
	public static function get_tags($name) {
		
		$paths = self::get_paths($name);
		
		$tags = array();
		
		foreach($paths as $path) {
			$ext = pathinfo($path, PATHINFO_EXTENSION);
			if(!$ext) continue;
			
			if($ext === 'js') array_push($tags, 
				sprintf('<script type="text/javascript" src="%s"></script>', BASE_URI.$path));
			else if($ext === 'css') array_push($tags, 
				sprintf('<link rel="stylesheet" href="%s" />', BASE_URI.$path));
		}
		
		return $tags;
	}
	
	public static function get_paths($name) {
		
		global $libraries;
		
		$name = pathinfo($name, PATHINFO_BASENAME);
		
		if(array_key_exists($name, $libraries)) {
			// A library
			$paths = $libraries[$name];
		} else {
			// Not a library
			$link = ContentManager::getResourceLink($name);
			if(!$link) return array();
			$paths = array($link);
		}
		
		return $paths;
	}
	
	public static function add_dependancies() {
		
		if(!isset($GLOBALS['view_config'])) return;
		
		$view_config = $GLOBALS['view_config'];
		
		foreach($view_config['dependancies'] as $dep) {
			
			$tags = self::get_tags($dep);
			
			foreach($tags as $tag) echo $tag."\n";
		}
	}
		
	public static function add_custom_head_content() {
		
		if(!isset($GLOBALS['view_config']['custom_head'])) return;
		
		foreach($GLOBALS['view_config']['custom_head'] as $custom_head) {
			
			echo $custom_head."\n";
		}
	}
	
	public static function add_bootscript() {
		
		$bootScript="
			var baseURI='%s';
		";
		
		$bootScript=sprintf($bootScript,BASE_URI);
		
		echo sprintf('<script type="text/javascript">%s</script>',$bootScript);
	}
	
}

?>
