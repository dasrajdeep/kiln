<?php

defined('SYSTEM_STARTED') or die('You are not permitted to access this resource.');

class ContentManager {
	
	private static $cache_location='cache/';
	
	private static $contentTypes=array(
		'graphics'=>array('jpg','jpeg','png','gif','bmp'),
		'stylesheets'=>array('css'),
		'scripts'=>array('js'),
		'fonts'=>array('eot','ttf','woff','svg','otf'),
		'all'=>array('jpg','jpeg','png','gif','bmp','svg','css','js','eot','ttf','woff')
	);
	
	public static function init() {}
	
	public static function getContentType($extension) {
		
		$keys = array_keys(self::$contentTypes);
		
		foreach($keys as $key) {
			if(in_array($extension, self::$contentTypes[$key])) return $key;
		}
	}
	
	public static function getResourceLink($resourceName) {
		
		$ext = pathinfo($resourceName, PATHINFO_EXTENSION);
		
		if(!$ext || !in_array($ext, self::$contentTypes['all'])) return '';
		
		$type = self::getContentType($ext);
		
		if($type === 'graphics') {
			$link = PATH_GRAPHICS.$resourceName;
		} else if($type === 'stylesheets') {
			$link = PATH_STYLES.$resourceName;
		} else if($type === 'scripts') {
			$link = PATH_SCRIPTS.$resourceName;
		} else if($type === 'fonts') {
			$link = PATH_FONTS.$resourceName;
		} else {
			$link = '';
		}
		
		if($link && file_exists(BASE_DIR.$link)) {
			CacheManager::moveToCache($link);
			return self::$cache_location.$resourceName;
		} else if($link && !file_exists(BASE_DIR.$link)) {
			return '';
		}
	}
	
	public static function serveContent($contentName) {
		
		global $libraries;
		
		$link = self::inLibrary($contentName);
		
		if($link === FALSE) {
			$link = self::getResourceLink($contentName);
			if($link) {
				if(CacheManager::lookupCache($contentName)) header('Location: '.BASE_URI.PATH_CACHE.$contentName);
				else header('HTTP/1.1 404 Not Found');
			} else header('HTTP/1.1 404 Not Found');
		} else {
			header('Location: '.BASE_URI.$link);
		}
	}
	
	public static function inLibrary($contentName) {
		
		global $libraries;
		
		foreach($libraries as $lib) {
			if(is_array($lib)) {
				foreach($lib as $path) {
					if($contentName === pathinfo($path, PATHINFO_BASENAME)) return $path;
				}
			} else {
				if($contentName === pathinfo($lib, PATHINFO_BASENAME)) return $lib;
			}
		}
		
		return FALSE;
	}
}

?>
