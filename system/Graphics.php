<?php

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

?>
