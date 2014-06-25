<?php

/**
 * ==========================================================
 * 						GLOBAL CONSTANTS
 * ==========================================================
 */

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

class CacheManager {
	
	const CONTENT_TYPE_GRAPHICS = "GRAPHIC";
	const CONTENT_TYPE_SCRIPT = "SCRIPT";
	const CONTENT_TYPE_STYLESHEET = "STYLESHEET";
	const CONTENT_TYPE_FONT = "FONT";
	
	private static $cacheRegistry=array();
	
	private static $persistenceTime=1800;
	
	public static function init() {}
	
	public static function moveToCache($resourcePath) {
		
		$filename = pathinfo($resourcePath, PATHINFO_BASENAME);
		
		if(file_exists(PATH_CACHE.$filename)) {
			$modifyTime = filemtime($resourcePath);
			$copyTime = filemtime(PATH_CACHE.$filename);
			if($modifyTime >= $copyTime) copy($resourcePath, PATH_CACHE.$filename);
		} else {
			copy($resourcePath, PATH_CACHE.$filename);
		}
		
		self::$cacheRegistry[$filename]=time();
		
		return true;
	}
		
	public static function removeFromCache($fileName) {
	
		if(file_exists(PATH_CACHE.$fileName)) unlink(PATH_CACHE.$fileName);
		
	}
	
	public static function lookupCache($fileName) {
	
		if(file_exists(PATH_CACHE.$fileName)) return PATH_CACHE.$fileName;
		else return null;
		
	}
	
	public static function removeOldItems() {
		
		$allowedAge=time()-self::$persistenceTime;
		
		foreach(array_keys(self::$cacheRegistry) as $resource) if(self::$cacheRegistry[$resource]<$allowedAge) unset(self::$cacheRegistry[$resource]);
	}
	
	public static function writeRegistry() {
		
		$regFileData='';
		
		foreach(array_keys(self::$cacheRegistry) as $resource) $regFileData=$resource.','.self::$cacheRegistry[$resource]."\n";
		
		file_put_contents(PATH_CACHE.'.cache',$regFileData);
	}
}

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

class Graphics {

    public static function createImageString($imageFile) {

        $imageInfo = getimagesize($imageFile);
        $imageString = file_get_contents($imageFile);

        if ($imageInfo)
            return $imageString;
    }

    public static function getMIME($imageFile) {

        $imageInfo = getimagesize($imageFile);

        return $imageInfo['mime'];
    }

    public static function resizeImageString($imageString, $mime, $width, $height) {

        $image = imagecreatefromstring($imageString);
        $image_width = imagesx($image);
        $image_height = imagesy($image);

        $temp = imagecreatetruecolor($width, $height);
        $background = imagecolorallocate($temp, 0, 0, 0);
        imagecolortransparent($temp, $background);
        imagealphablending($temp, false);
        imagecopyresampled($temp, $image, 0, 0, 0, 0, $width, $height, $image_width, $image_height);
        imagesavealpha($temp, true);
        $image = $temp;

        ob_start();
        $type = substr($mime, 6);
        if ($type === 'jpeg')
            imagejpeg($image);
        else if ($type === 'gif')
            imagegif($image);
        else if ($type === 'png')
            imagepng($image);
        $image = ob_get_clean();

        return $image;
    }
    
    public static function resizeImageStringByFactor($imageString, $mime, $factor) {
        
        $image = imagecreatefromstring($imageString);
        $image_width = imagesx($image);
        $image_height = imagesy($image);
        
        $required_width = $image_width * $factor;
        $required_height = $image_height * $factor;
        
        return self::resizeImageString($imageString, $mime, $required_width, $required_height);
    }
	
	public static function storeGraphics($imageFile) {
		
		$mime = self::getMIME($imageFile);
		$original = self::createImageString($imageFile);
		$filesize = filesize($imageFile);
		$extension = pathinfo($imageFile, PATHINFO_EXTENSION);
		
		$create_table = "CREATE TABLE IF NOT EXISTS kiln_graphics (
			id INT NOT NULL AUTO_INCREMENT,
			content MEDIUMBLOB NOT NULL,
			mime VARCHAR(255),
			filesize INT,
			extension VARCHAR(255),
			PRIMARY KEY (id)	
		) engine=InnoDB";
		
		R::exec($create_table);
		
		// Check for concurrency issues
		$last_id = R::getAssocRow("SELECT MAX(id) AS last_id FROM kiln_graphics");
		$last_id = $last_id[0]['last_id'] + 1;
		
		$query = "INSERT INTO kiln_graphics (id,content,mime,filesize,extension) VALUES(:id,:content,:mime,:filesize,:extension)";
		
		$result = R::exec($query, array(
			':id'=>$last_id,
			':content'=>$original,
			':mime'=>$mime,
			':filesize'=>$filesize,
			':extension'=>$extension
		));
		
		if($result) return $last_id;
		else return 0;
	}
	
	public static function fetchGraphics($graphicsID) {
		
		$create_table = "CREATE TABLE IF NOT EXISTS kiln_cache (
			filename VARCHAR(255) NOT NULL,
			type VARCHAR(255),
			ref INT,
			expiry INT NOT NULL,
			PRIMARY KEY (filename)
		) engine=InnoDB";
		
		R::exec($create_table);
		
		$query = "SELECT filename,expiry FROM kiln_cache WHERE type=:type AND ref=:ref";
		
		$row = R::getAssocRow($query, array(':type'=>CacheManager::CONTENT_TYPE_GRAPHICS, ':ref'=>$graphicsID));
		
		if(count($row)) {
			$row = $row[0];
			if($row['expiry'] < time()) {
				// recache
				$new_file = self::cacheGraphics($graphicsID);
				if($new_file === FALSE) return FALSE;
				$query = "UPDATE kiln_cache SET filename=:new_file,expiry=:expiry WHERE filename=:old_file";
				R::exec($query, array(':new_file'=>$new_file, ':old_file'=>$row['filename'], ':expiry'=>(time() + 60*60)));
				@unlink(PATH_CACHE.$row['filename']);
				return PATH_CACHE.$new_file;
			} else return PATH_CACHE.$row['filename'];
		} else {
			// cache first time
			$filename = self::cacheGraphics($graphicsID);
			if($filename === FALSE) return FALSE;
			$query = "INSERT INTO kiln_cache (filename,type,ref,expiry) VALUES(:filename,:type,:ref,:expiry)";
			R::exec($query, array(':filename'=>$filename, ':type'=>CacheManager::CONTENT_TYPE_GRAPHICS, ':ref'=>$graphicsID, ':expiry'=>(time() + 60*60)));
			return PATH_CACHE.$filename;
		}
	}

	public static function deleteGraphics($graphicsID) {
		
		$query = "SELECT filename FROM kiln_cache WHERE type=:type AND ref=:ref";
		
		$row = R::getAssocRow($query, array(':type'=>CacheManager::CONTENT_TYPE_GRAPHICS, ':ref'=>$graphicsID));
		
		if(!count($row)) return FALSE;
		
		$row = $row[0];
		
		@unlink(PATH_CACHE.$row['filename']);
		
		R::exec("DELETE FROM kiln_cache WHERE filename=:filename", array(':filename'=>$row['filename']));
		R::exec("DELETE FROM kiln_graphics WHERE id=:id", array(':id'=>$graphicsID));
		
		return TRUE;
	}
	
	private static function cacheGraphics($graphicsID) {
			
		$query = "SELECT content,mime,extension FROM kiln_graphics WHERE id=:id";
		
		$row = R::getAssocRow($query, array(':id'=>$graphicsID));
		
		if(!count($row)) return FALSE;
		
		$row = $row[0];
		
		$filename = null;
		
		while(!$filename) {
			$filename = generate_random_string().'.'.$row['extension'];
			if(file_exists(PATH_CACHE.$filename)) $filename = null;
		}
		
		file_put_contents(PATH_CACHE.$filename, $row['content']);
		
		return $filename;
	}
	
}

class Helper {
	
	private static $registry=array();
	
	public static function init() {
		
		self::$registry=parse_ini_file(PATH_HELPERS.'.helpers',true);
		
	}
	
	public static function __callStatic($name, $arguments) {
		
		if(isset(self::$registry['helper_registry'][$name])) {
			require_once(PATH_HELPERS.self::$registry['helper_registry'][$name]);
			return call_user_func_array($name,$arguments);
		} else trigger_error('Called helper method does not exist.',512);
	}
	
	public static function getContentLink($contentName) {
		
		$rel_link = ContentManager::getResourceLink($contentName);
		
		return BASE_URI.$rel_link;
	}

	public static function addViewComponent($componentName, $var=null) {
		
		if(!isset($GLOBALS['view_registry'])) {
			$reg=parse_ini_file(PATH_VIEWS.'.views',true);
			$GLOBALS['view_registry']=$reg['view_registry'];
		}
		
		if(!isset($GLOBALS['view_registry'][$componentName])) return false;
		
		$path=$GLOBALS['view_registry'][$componentName];
		
		if(file_exists($path)) {
			include($path);
			return true;
		} else return false;
	}

	public static function addDependancy($dependancyName) {
		
		if(!isset($GLOBALS['view_config'])) $GLOBALS['view_config'] = array('dependancies'=>array());
		
		$dependancies = array_filter(explode(',', $dependancyName));
		
		foreach($dependancies as $dep) array_push($GLOBALS['view_config']['dependancies'], $dep);
	}
	
	public static function addCustomHeadContent($content) {
		
		if(!isset($GLOBALS['view_config'])) $GLOBALS['view_config']=array('dependancies'=>array());
		
		if(!isset($GLOBALS['view_config']['custom_head'])) $GLOBALS['view_config']['custom_head'] = array();
		
		array_push($GLOBALS['view_config']['custom_head'], $content);
	}
	
	public static function setCompleteView()  {
		
		$GLOBALS['view_type']='complete';
	}
	
}

class Mailer {
	
	private static $spool_location = 'data/_spool';
	
	private static $host = null;
	private static $port = null;
	private static $username = null;
	private static $password = null;
	private static $sender_id = null;
	private static $sender_name = null;
	
	public static function init() {
		self::$host = Registry::lookupConfig(Registry::CONFIG_TYPE_MAIL, 'smtp_host');
		self::$port = Registry::lookupConfig(Registry::CONFIG_TYPE_MAIL, 'smtp_port');
		self::$username = Registry::lookupConfig(Registry::CONFIG_TYPE_MAIL, 'smtp_username');
		self::$password = Registry::lookupConfig(Registry::CONFIG_TYPE_MAIL, 'smtp_password');
		self::$sender_id = Registry::lookupConfig(Registry::CONFIG_TYPE_MAIL, 'sender_email');
		self::$sender_name = Registry::lookupConfig(Registry::CONFIG_TYPE_MAIL, 'sender_name');
	}
	
	public static function sendMail($recipient, $messageText, $messageHTML, $subject) {
		
		$transport = Swift_SmtpTransport::newInstance(self::$host, self::$port)
		  ->setUsername(self::$username)
		  ->setPassword(self::$password);
		
		$mailer = Swift_Mailer::newInstance($transport);
		
		$message = Swift_Message::newInstance($subject)
		  ->setFrom(array(self::$sender_id => self::$sender_name))
		  ->setTo(array($recipient))
		  ->setBody($messageText, 'text/plain')
		  ->addPart($messageHTML, 'text/html');
		
		$result = $mailer->send($message);
		
		return $result;
	}
	
	public static function spoolMail($recipient, $messageText, $messageHTML, $subject) {
		
		$spool = new Swift_FileSpool(self::$spool_location);
		
		$transport = Swift_SpoolTransport::newInstance($spool);
		$mailer = Swift_Mailer::newInstance($transport);
		
		$message = Swift_Message::newInstance($subject)
		  ->setFrom(array(self::$sender_id => self::$sender_name))
		  ->setTo(array($recipient))
		  ->setBody($messageText, 'text/plain')
		  ->addPart($messageHTML, 'text/html');
		  
		$result = $mailer->send($message);
		
		return $result;  
	}
	
	public static function sendSpooledMails() {
		
		$spool = new Swift_FileSpool(self::$spool_location);
		
		$spoolTransport = Swift_SpoolTransport::newInstance($spool);
		
		$realTransport = Swift_SmtpTransport::newInstance(self::$host, self::$port)
		  ->setUsername(self::$username)
		  ->setPassword(self::$password);
		  
	  	$spool = $spoolTransport->getSpool();
		$spool->setMessageLimit(10);
		$spool->setTimeLimit(100);
		
		$sent = $spool->flushQueue($realTransport);
		
		if($sent == 1) echo "Spooled mails sent.\n";
		
		return $sent;
	}
	
}

class Registry {
	
	const CONFIG_TYPE_DATABASE = "DATABASE";
	const CONFIG_TYPE_MAIL = "MAIL";
	const CONFIG_TYPE_APP = "APP";
	const PORT_TYPE_PUBLIC = "PUBLIC";
	const PORT_TYPE_PRIVATE = "PRIVATE";
	
	/**
	 * Contains registry entries of the form:
	 * (<controller_name>,<method_name>)
	 */
	private static $port_registry=array();
	
	/**
	 * Contains registry entries of the form:
	 * <type, key>=<value>
	 */
	private static $app_config=array();
	
	/**
	 * Contains registry entries of the form:
	 * <key>=<value>
	 */
	private static $custom_config = array();
	
	public static function init() {
		
		if(!file_exists(BASE_DIR.'app/config/app.ini')) {
			copy(BASE_DIR.'app/config/app.ini.default', BASE_DIR.'app/config/app.ini');
		}
		
		$config_app = parse_ini_file(BASE_DIR.'app/config/app.ini', TRUE);
		$config_ports = parse_ini_file(BASE_DIR.'app/config/ports.ini', TRUE);
		$config_custom = parse_ini_file(BASE_DIR.'app/config/custom.ini', FALSE);
		
		self::$port_registry = $config_ports;
		
		foreach(array_keys($config_ports['PUBLIC']) as $key) {
			$value = trim($config_ports['PUBLIC'][$key]);
			if($value) self::$port_registry['PUBLIC'][$key] = explode(':', $value);
			else self::$port_registry['PUBLIC'][$key] = null;
		}
		
		foreach(array_keys($config_ports['PRIVATE']) as $key) {
			$value = trim($config_ports['PRIVATE'][$key]);
			if($value) self::$port_registry['PRIVATE'][$key] = explode(':', $value);
			else self::$port_registry['PRIVATE'][$key] = null;
		}
		
		self::$app_config = $config_app;
		self::$custom_config = $config_custom;
	}
	
	public static function portExists($portName) {
		
		if(array_key_exists($portName, self::$port_registry['PUBLIC'])) return TRUE;
		if(array_key_exists($portName, self::$port_registry['PRIVATE'])) return TRUE;
		
		return FALSE;
	}
	
	public static function lookupPort($portName) {
		
		if(isset(self::$port_registry['PUBLIC'][$portName])) return array_merge(self::$port_registry['PUBLIC'][$portName], array('PUBLIC'));
		else if(isset(self::$port_registry['PRIVATE'][$portName])) return array_merge(self::$port_registry['PRIVATE'][$portName], array('PRIVATE'));
		else return null;
	}
	
	public static function listPorts() {
		
		return array_merge(array_keys(self::$port_registry['PUBLIC']), array_keys(self::$port_registry['PRIVATE']));
	}
	
	public static function lookupConfig($type, $configKey) {
		
		if(isset(self::$app_config[$type][$configKey])) return self::$app_config[$type][$configKey];
		else return null;
	}
	
	public static function lookupCustomConfig($key) {
		
		if(array_key_exists($key, self::$custom_config)) return self::$custom_config[$key];
		else return null;
	}
	
}

class Session {
    
    /**
     * Contains information regarding whether a session is authorized or not.
     * 
     * @var boolean
     */
    private static $authorized=false;
    
    /**
     * Contains information regarding whether a session is running or not.
     * 
     * @var boolean
     */
    private static $running=false;
    
    /**
     * Contains the session timeout in seconds.
     * 
     * @var int
     */
    private static $timeout=1800;
    
    /**
     * Contains the session control keys.
     * 
     * @var string[]
     */
    private static $control_keys=array('session_user','session_vars','expire_time');
	
	/**
	 * Initializes the session manager.
	 */
    public static function init() {
        if(isset($_SESSION['session_user'])) {
            if(self::timedOut()) {
                self::stop();
                return;
            }
            self::$running=TRUE;
        } else $_SESSION=array();
    }
    
    /**
     * Tells whether a session is running or not.
     * 
     * @return boolean
     */
    public static function isRunning() {
        return self::$running;
    }
    
    /**
     * Starts a session.
     * 
     * @param string $id
     */
    public static function start($id) {
        $_SESSION['session_user']=$id;
        $_SESSION['session_vars']=array();
        self::setTimeout(self::$timeout);
        self::$running=TRUE;
    }
    
    /**
     * Stops a running session.
     */
    public static function stop() {
        $_SESSION=array();
        session_destroy();
        self::$running=FALSE;
    }
    
    /**
     * Fetches the user ID associated with a session.
     * 
     * @return string
     */
    public static function getUserID() {
        if(isset($_SESSION['session_user'])) return $_SESSION['session_user'];
		else return null;
    }
    
    /**
     * Sets a variable for the current running session.
     * 
     * @param string $key
     * @param string $value
     * @return boolean
     */
    public static function setVar($key,$value) {
		if(!self::$running) return false;
		$_SESSION['session_vars'][$key]=$value;
		return true;
    }
    
    /**
     * Fetches a variable associated with the running session.
     * 
     * @param string $key
     * @return string|null
     */
    public static function getVar($key) {
		if(!self::$running) return null;
        if(isset($_SESSION['session_vars'][$key])) return $_SESSION['session_vars'][$key];
		else return null;
    }
    
    /**
     * Fetches the session ID for the current session.
     * 
     * @return string
     */
    public static function getSessionID() {
        return session_id();
    }
    
    /**
     * Sets the timeout for the session.
     * 
     * @param int $minutes
     * @return boolean
     */
    public static function setTimeout($minutes) {
        if($minutes<5) return FALSE;
        $seconds=$minutes*60;
        $_SESSION['expire_time']=time()+$seconds;
        return TRUE;
    }
    
    /**
     * Tells whether the session has timed out or not.
     * 
     * @return boolean
     */
    public static function timedOut() {
        $expiry=$_SESSION['expire_time'];
        if(time()>$expiry) return TRUE;
        return FALSE;
    }
    
    /**
     * Sets the current session as authorized or unauthorized.
     * 
     * @param boolean $auth
     */
    public static function setAuthorized($auth) {
        self::$authorized=$auth;
    }
    
    /**
     * Tells whether the current session is authorized or not.
     * 
     * @return boolean
     */
    public static function isAuthorized() {
        return self::$authorized;
    }
}

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
		
		$scripts = array();
		$styles = array();
		
		foreach($view_config['dependancies'] as $dep) {
			
			$paths = self::get_paths($dep);
			
			foreach($paths as $path) {
				$ext = pathinfo($path, PATHINFO_EXTENSION);
				if($ext === 'js') {
					array_push($scripts, $path);
				} else if($ext === 'css') {
					array_push($styles, $path);
				}
			}
			
			/*$tags = self::get_tags($dep);
			
			foreach($tags as $tag) echo $tag."\n";*/
		}
		
		$script_path = Optimizer::packScripts($scripts);
		$styles_path = Optimizer::packStylesheets($styles);
		
		if($script_path) echo "\n".sprintf('<script type="text/javascript" src="%s"></script>', BASE_URI.$script_path)."\n";
		if($styles_path) echo "\n".sprintf('<link rel="stylesheet" href="%s" />', BASE_URI.$styles_path)."\n";
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

CacheManager::init();
ContentManager::init();
Helper::init();
Mailer::init();
Registry::init();
Session::init();

?>