<?php

// Add custom script path
class Optimizer {
	
	public static function packScripts($list) {
		
		if(!count($list)) return null;
		
		$content = '';
		
		foreach($list as $item) {
			$content = $content . file_get_contents($item) . "\n\n";
		}
		
		$cache_file = PATH_CACHE.generate_random_string().'.js';
		
		file_put_contents($cache_file, $content);
		
		return $cache_file;
	}
	
	public static function packStylesheets($list) {
		
		if(!count($list)) return null;
		
		$content = '';
		
		foreach($list as $item) {
			$content = $content . file_get_contents($item) . "\n\n";
		}
		
		$cache_file = PATH_CACHE.generate_random_string().'.css';
		
		file_put_contents($cache_file, $content);
		
		return $cache_file;
	}
	
	public static function composeView($view) {
		
		$view_registry = parse_ini_file(PATH_VIEWS.'.views',true);
		$view_registry = $view_registry['view_registry'];
		
		if(!isset($view_registry[$view])) return FALSE;
		
		$content = file_get_contents($view_registry[$view]);
		
		$page = FALSE;
		
		$regex_fragment = "/Helper::addViewComponent[(]([^(]+)[)]\\s*;/";
		$regex_dependancy = "/Helper::addDependancy\(([^(]+)\)\\s*;/"; 
		
		if(preg_match("/Helper::setCompleteView\(\);/", $content)) $page = TRUE;
		preg_match_all($regex_fragment, $content, $view_components);
		preg_match_all($regex_dependancy, $content, $dependancies);
		
		$content = str_replace("Helper::setCompleteView();", "", $content);
		
		$count = 0;
		while(count($view_components[0])) {
			$matches = $view_components[0];
			$fragments = $view_components[1];
			for($index = 0; $index < count($matches); $index++) {
				$count++;
				$args = explode(",", $fragments[$index]);
				$frag = trim($args[0]);
				$frag = substr($frag, 1, strlen($frag) - 2);
				if(!isset($view_registry[$frag])) continue;
				$file = $view_registry[$frag];
				$frag = file_get_contents($file);
				$frag = str_replace('$var', '$var'.$count, $frag);
				$pos = strpos($content, $matches[$index]);
				$pos = strpos($content, '?>', $pos) + 2;
				$content = substr($content, 0, $pos) . $frag . substr($content, $pos);
				if(isset($args[1]) && trim($args[1])) $content = str_replace($matches[$index], '$var'.$count.' = '.trim($args[1]).';', $content);
				else $content = str_replace($matches[$index], '', $content);
			}
			preg_match_all($regex_fragment, $content, $view_components);
		}
		
		$deps = array();
		
		if(count($dependancies[0])) {
			$list = $dependancies[1];
			$matches = $dependancies[0];
			for($index = 0; $index < count($matches); $index++) {
				$dep = $list[$index];
				$dep = explode(",", substr($dep, 1, strlen($dep) - 2));
				$deps = array_merge($deps, $dep);
				$content = str_replace($matches[$index], '', $content);
			}
		}
		if($page) {
			$container = file_get_contents('core/container.php');
			$container = str_replace('</head>', 
				sprintf('<script type="text/javascript"> var baseURI = \'%s\';</script>', BASE_URI) . '</head>', 
				$container);
			$scripts = array();
			$styles = array();
			foreach($deps as $dep) {
				$paths = ViewManager::get_paths($dep);
			
				foreach($paths as $path) {
					$ext = pathinfo($path, PATHINFO_EXTENSION);
					if($ext === 'js') {
						array_push($scripts, $path);
					} else if($ext === 'css') {
						array_push($styles, $path);
					}
				}
			}
			$script_path = Optimizer::packScripts($scripts);
			$styles_path = Optimizer::packStylesheets($styles);
			if($script_path) $container = str_replace('</head>', 
				sprintf('<script type="text/javascript" src="%s"></script>', BASE_URI.$script_path) . '</head>', 
				$container);
			if($styles_path) $container = str_replace('</head>', 
				sprintf('<link rel="stylesheet" href="%s" />', BASE_URI.$styles_path) . '</head>', 
				$container);
			$container = str_replace('ViewManager::add_bootscript();', '', $container);
			$container = str_replace('ViewManager::add_dependancies();', '', $container);
			$container = str_replace('ViewManager::add_custom_head_content();', '', $container);
			$container = str_replace('<?php if(isset($html_body)) echo $html_body; ?>', '', $container);
			$container = str_replace('<?php defined(\'SYSTEM_STARTED\') or die(\'You are not permitted to access this resource.\'); ?>', 
				"", $container);
			$container = str_replace('<?php echo Registry::lookupConfig(Registry::CONFIG_TYPE_APP, \'title\'); ?>', 
				Registry::lookupConfig(Registry::CONFIG_TYPE_APP, 'title'), $container);
			$container = str_replace('BASE_DIR', "'".BASE_DIR."'", $container);
			
			$container = str_replace('<body>', '<body>'.$content, $container);
			
			$container = preg_replace("/<\?php(\\s*)\?>/", "", $container);
			
			$html = $container;	
		} else $html = $content;
		
		file_put_contents('production/'.$view.'.inc', $html);
		
		return true;
	}
	
	public static function analyzeFlow() {
		
		$script = $_SERVER['SCRIPT_NAME'];
		$params = implode("_", array_keys($_REQUEST));
		echo $script.'.'.$params;
		
		echo sprintf("<pre>%s</pre>", print_r(debug_backtrace(), TRUE));
	}
	
	public static function refactorFlows() {
		
		$ports = Registry::listPorts();
		
		foreach($ports as $port) {
			$base_uri = $GLOBALS['env_base_uri'];
			
			$content = sprintf('<?php
				define("SYSTEM_STARTED", TRUE);
				$env_base_uri = "%s"; 
			?>', $base_uri);
			
			$content .= file_get_contents(BASE_DIR.'system.inc');
			$content .= file_get_contents(BASE_DIR.'index.inc');
			
			$config = Registry::lookupPort($port);
			$controller = $config[0];
			$method = $config[1];
			
			$controller = file_get_contents(BASE_DIR.'app/controllers/'.$controller.'Controller.php');
			
			$content = "\n" . $content . $controller;
			
			file_put_contents(BASE_DIR.'production/'.$port.'.php', $content);
		}
	}
	
	public static function optimize() {
		
		self::refactorFlows();
		
		$view_registry = parse_ini_file(PATH_VIEWS.'.views',true);
		$view_registry = $view_registry['view_registry'];
		
		foreach(array_keys($view_registry) as $view) {
			self::composeView($view);
		}
	}
	
}

?>