<?php
function fz_songbook_substitute($str, $filter, $format, $langcode) {
  static $x = 0;
  $x++;
  $mf = array ();
  $ms = array ();
  $f_minta = '#\[songbook\|[^]]*\]#';
  $s_minta = "/\[songbook\].*\[\/songbook\]/simx";
    
  $is_f = preg_match ( $f_minta, $str, $mf );
  $is_s = preg_match ( $s_minta, $str, $ms );
  
  if ($is_f === 0 && $is_s === 0) {
    return $str;
  }
  
  if ($is_f > 0) {
    $match = $mf [0];
    $s = array ("[Songbook", "|", "]" );
    $r = array ("", "", "" );
    $match = str_ireplace ( $s, $r, $match );
    $match = str_replace("\\","/",$match);
    
    $sb_files = variable_get("fz_songbook_files","public://");
    $sb_files = strtolower($sb_files);
    switch($sb_files ){
      case "public://":
      case "private://":
        $pre = drupal_realpath( $sb_files );
        break;
      case "<root>":
        $pre = DRUPAL_ROOT;
        break;
      default:
        $pre = "";
    }
    
    //Is there absolute path in the songbook "tag"?
    if ( (substr($match,0,1) == "/") ||  (substr($match,1,2) ==':/') ) {
      $filename = $match;
    }else{
      $filename = str_replace ( "\\", "/", $pre) . "/" . $match;
    }
    //Is there the file?
    if (file_exists ( $filename )) {
      $txt = file_get_contents ( $filename );
      $sgb = fz_songbook_convert ( $txt, "asc" );
    } else {
      $sgb = "The file does not exists: " . $match;
      drupal_set_message ( $sgb, "warning" );
      $sgb = '<div class="warning messages">' . $sgb . "</div>";
    }
    
    $str = preg_replace ( $f_minta, $sgb, $str, 1 );
    //Recursive call the same function
    if(preg_match($f_minta,$str,$mf)>0){
      $str = fz_songbook_substitute($str,"","","");
    }
  }
  
  if (($is_s > 0)) {
    $match = $ms [0];
    $s = array ("[songbook]","[/songbook]");
    $r = array ("","","");
    $match = str_ireplace ( $s, $r, $match );
    
    $txt = "";
    $sgb = fz_songbook_convert ( $match, "html" );
    $str = preg_replace ( $s_minta, $sgb, $str, 1 );
  }
  $mp = drupal_get_path ( 'module', 'fz_songbook' );
  drupal_add_css ( $mp . "/fz_songbook.css" );
  return $str;
}

/**
 * Convert a text to songbook format
 *
 * @param string $txt
 * @return string - text with chords
 */
function fz_songbook_convert($txt, $format = "asc") {
  if ($format == "asc") {
    $txt = str_ireplace ( "<br>\n", "\n", $txt );
    $txt = str_ireplace ( "<p>", "", $txt );
    $txt = str_ireplace ( "</p>", "\n\n", $txt );
  } else {
    $txt = str_ireplace ( "<br>", "\n", $txt );
    $txt = preg_replace ( "/<br[^\/]*\/>/", "", $txt );
    $txt = str_ireplace ( "<p>", "", $txt );
    $txt = str_ireplace ( "</p>", "\n\n", $txt );
  }
  
  $title = '';
  $header = "";
  $header .= _songbook_title ( $txt, $title );
  $header .= _songbook_subtitle ( $txt );
  $header .= _songbook_key ( $txt );
  $header .= _songbook_tag ( $txt, $title );
  $header .= _songbook_time ( $txt );  
  $header .= _songbook_musicpath ( $txt );
  // $header = preg_replace('/<br[^>]*>/i',"",$header);str_replace('<br>','',$header);
  $chordstyle = variable_get ( "fz_songbook_chord", "color: blue;" );
  $a = explode ( "\n", $txt );
  $out = "";
  foreach ( $a as $e ) {
    if (strlen ( $e ) > 0) {
      $lines = _songbook_convert_row ( $e, $chordstyle );
      if (! empty ( $lines [0] )) {
        $out .= $lines [0] . "\n";
      }
      $out .= $lines [1] . "\n";
    }
  }
  _soc_eoc ( $out );
  _sot_eot ( $out );
  _soh_eoh ( $out );
  _sog_eog ( $out );
  $out = str_replace ( "&nbsp;", "", $out );
  $out = str_replace ( array (
      "<br>",
      "<br />"
  ), "", $out );
  $out = $header . '<pre class="sb_text">' . $out . "</pre>";
  return $out;
}

/*
 * Convert a line of songbook
 * return two lines:
 * - The first line is the line of chords,
 * - The second line is the line of text
 */
function _songbook_convert_row($e, $chordstyle) {
  // A hidden row
  if (_songbook_hidden ( $e )) {
    return array ( "", "" );
  }
  
  // there is nothing else in row of comment
  if (_songbook_comment ( $e )) {
    return array ( "", $e );
  }
  // Search the '[' sign in the row
  if (strpos ( $e, "[" ) === false) {
    return array ( "" , $e );
  }
  
  $e = str_replace ( "&nbsp;", " ", $e );
  $e = trim ( $e );
  
  $chord = ""; // String of Chords
  $text  = ""; // string only text, no chords

  // length of chord
  $cw = 0;
  // search the first '['
  $a = explode ( "[", $e, 2 );
  $f = $e;
  while ( count ( $a ) > 1 ) {
    $text .= $a[0]; // text brefore '[' 
    $db = iconv_strlen ( $a[0] ); // number of characters before '[' 

    if ($a[0] == " ") {
      $onlyspace = true;
    } else {
      $onlyspace = false;
    }
	if(empty($a[0]) ) 
	  $nostring = true; 
	else 
	  $nostring = false;

    $b = explode ( "]", $a[1], 2 ); // after '[' search for ']' 
    if (count ( $b ) > 1) {
      // then the length os string in the row
      $spdb = ($db > 0 ? $db - $cw : 0);

	  if ($onlyspace)
        $spdb++;

	  if($spdb<0) 
		  $spdb = 0;

	  if($spdb<0){
		  fz_t($b);
		  fz_t($spdb);
		  fz_t($cw);
	  }

	  $chord .= str_repeat ( " ", $spdb ) . $b[0]; // this is the string of chord
      $cw     = iconv_strlen ( $b[0] ); // length of last schord
      $e      = $b[1]; // string on the right side
    } else {
      $e = $b[0];
    }
    $a = explode ( "[", $e, 2 ); // search for the next '['
  }
  $text .= $a[0];
  
  if (strlen ( $chord ) > 0)
    $chord = '<span class="sb_chord"' . (! empty ( $chordstyle ) ? ' style="' . $chordstyle . '"' : '') . '>' . $chord . '</span>';
  
  // If the chord is between repeat sign or only chord, or vagy csak akkordok vagy csak beat sign
  $p1 = str_replace ( array ( "||:", ":||", "|" ), "", $text );
  
  if (strlen ( trim ( $p1 ) ) == 0) {
    $text = $f;
    $text = str_replace ( array (
        "[",
        "]"
    ), array (
        '<span class="sb_chord"' . (! empty ( $chordstyle ) ? ' style="' . $chordstyle . '"' : '') . '>',
        '</span>'
    ), $text );
    $chord = "";
  }
  
  return array (
      $chord,
      $text
  );
}

/**
 *
 * @param string $e
 * @return unknown
 */
function _songbook_title(&$e, &$t) {  
  $title = "";
  $m = array ();
  $e = str_ireplace("{title:","{t:",$e);
  $is = preg_match ( "/{t:[^}]*}/simx", $e, $m );
  
  if ($is > 0) {
    $e = preg_replace ( "/{t:[^}]*}/simx", EOL, $e );
    $title = str_replace ( array (
        "{t:",
        "}"
    ), array (
        "",
        ""
    ), $m [0] );
    $e = _songbook_eol ( $e );
  }    
  $stitle = variable_get ( 'fz_songbook_t', 'font-size: 2em;' );
  return '<center><h2 class="sb_title" style="' . $stitle . '">' . $title . '</h2></center>';
}
/**
 * delete an end of line character
 *
 * @param unknown $txt
 * @param string $j
 * @return mixed
 */
function _songbook_eol($e) {
  $e = str_replace ( array (
      EOL . "\r\n",
      EOL . "\n"
  ), "", $e );
  $e = str_replace ( array (
      EOL . "<br>",
      EOL . "\n"
  ), "", $e );
  $e = str_replace ( EOL . "\n", "\n", $e );
  return $e;
}

/**
 *
 * @param string $e
 * @return unknown
 */
function _songbook_subtitle(&$e) {
  $ssubtitle = variable_get ( 'fz_songbook_st', 'font-size: 1.5em;' );
  $title = "";
  $m  = array ();
  $e  = str_ireplace("{subtitle:", "{sb:",$e);
  $is = preg_match ( "/{st:[^}]*}/simx", $e, $m );
  if ($is > 0) {
    $e = preg_replace ( "/{st:[^}]*}/simx", EOL, $e );
    $e = _songbook_eol ( $e );
    $title = str_replace ( array ( "{st:", "}"), array ( "",""), $m[0] );
  }
  return '<center><h3 class="sb_subtitle" style="' . $ssubtitle . '">' . $title . '</h3></center>';
}
/**
 *
 * @param string $e
 * @return string
 */
function _songbook_key(&$e) {
  $title = "";
  $m = array ();
  $is = preg_match ( "/{key:[^}]*}/simx", $e, $m );
  if ($is > 0) {
    $e = preg_replace ( "/{key:[^}]*}/simx", EOL, $e );
    $e = _songbook_eol ( $e );
    $title = '<span class="field field-label sb_key">' . t ( "Key" ) . ': </span>' . str_replace ( array (
        "{key:",
        "}"
    ), array (
        "",
        ""
    ), $m [0] ) . "<br/>";
  }
  return $title;
}



/**
 * Add new tag to taqxonomy vocabulary
 *
 * @param unknown $tag
 */
function _songbook_taxadding($tag, $title) {
  if (! _songbook_tag_exists ( $tag, 'songbook_' . $title )) {
    $vocab = taxonomy_vocabulary_machine_name_load ( SONGBOOK_VOCAB );
    taxonomy_term_save ( ( object ) array (
        'name' => $tag,
        'vid' => $vocab->vid,
        'description' => 'songbook_' . $title,
        'format' => 'filtered_html'
    ) );
  }
}

/**
 * Is there the tag in the taxonomy database
 *
 * @param string $tag
 * @param string $title
 * @return boolean
 */
function _songbook_tag_exists( $tag= '', $title = '') {
  $ok = false;
  $vid = taxonomy_get_term_by_name ( $tag, SONGBOOK_VOCAB );
  foreach ( $vid as $v ) {
    if ($v->description == $title) {
      $ok = true;
    }
  }
  return $ok;
}

/**
 * Show the time
 *
 * @param string $e
 * @return string
 */
function _songbook_time(&$e) {
  $title = "";
  $m = array ();
  $is = preg_match ( "/{time:[^}]*}/simx", $e, $m );
  if ($is > 0) {
    $e = preg_replace ( "/{time:[^}]*}/simx", '', $e );
    // $e = _songbook_eol($e);
    $title = '<span class="field field-label sb_tag">' . t ( "Time" ) . ':</span>' . str_replace ( array (
        "{time:",
        "}"
    ), array (
        "",
        ""
    ), $m [0] ) . "<br/>";
    ;
  }
  return $title;
}
/**
 * Show TAGS in the songbook
 *
 * @param string $e
 * @return string
 */
function _songbook_tag(&$e, $t) {
    $title = "";
    $m = array ();
    $is = preg_match_all( "/{tag:[^}]*}/imx", $e, $m );
    if($is>0){
        $e = preg_replace ( "/{tag:[^}]*}/imx", EOL, $e );
        
        foreach($m[0] AS $i => $mp){
          $e = _songbook_eol ( $e );
          $x = str_replace ( array ("{tag:","}"), "", $mp); 
          if (module_exists ( "taxonomy" )) {
              $is_tax = variable_get ( "fz_songbook_vocab", true );
              if ($is_tax) {
                  _songbook_taxadding ( $x, $t );
              }
              $title .= "<a class='sb_tag' href='tags/" . $x . "'>" . $x . "</a>,&nbsp;";
          }else{
              $title .= "<span class='sb_tag' >" . $x . "</span>,&nbsp;";
          }
          
        }
    }
    if(substr($title,-7) == ",&nbsp;"){
        $title = substr($title,0,-7);
    }   
    return '<span class="field field-label sb_tag">Tags:</span>' . $title . "<br/>";
}

/**
 * Show the musicpath
 *
 * @param string $e
 * @return string
 */
function _songbook_musicpath(&$e) {
  $title = "";
  $m = array ();
  $m1= array();
  $m2 = array();
  //fz_t($e);
  $is = preg_match_all( "/{musicpath:[^}]*}/imx", $e, $m );
  if ($is > 0) {
      $e = preg_replace ( "/{musicpath:[^}]*}/imx", '', $e );
      foreach($m[0] as $i => $mp){
          $x = str_replace ( array ("{musicpath:","}" ), "", $mp );  
          $regex = "/^(https:|http:|ftp:)\/\//i";
          $web = preg_match($regex, $x, $m1);
          //ftp:// http:// httpd://
          if($web>0){
              $a = $x;
          //Local filesystem in Windowson
          }else if(strpos($x, ":")==1){
              $a = $x;
          }else{
              //If SimplePhotoalbum exists
              if(module_exists("smplphotoalbum")){
                  $smp = preg_match("/smplphotoalbum\/v\.php\?/simx",$x,$m2);
                  if($smp>0 ){
                      if(strpos($x, "/") === 0 ){
                        $x = substr($x,1);
                      }
                      $a = pathinfo($x,PATHINFO_BASENAME);
                      //fz_die($x) ;
                  }else{
                      $a = pathinfo($x,PATHINFO_BASENAME);
                      if(strpos($x, "file://") === 0){  //file://-rel kezdõdik az elérési útvonal
                          $x = str_replace("file://",variable_get("fz_songbook_files","public://songbook"),$x);
                      }
                      if(strpos($x, "public://")===0){
                          $x = str_replace('public://','',$x);
                          $wrapper = file_stream_wrapper_get_instance_by_uri('public://');
                          $x = $wrapper->getExternalUrl()."/".$x;                         
                      }else{
                          $x = drupal_get_path('module', 'smplphotoalbum')."/v.php?i=".$x;
                      }
                  }
              //There is no Simple Photoalbum
              }else{
                  if (pathinfo($x,PATHINFO_BASENAME) == $x){
                      $p=variable_get("fz_songbook_files","public://songbook");
                      if(strpos($x, "public://")===0){
                        $wrapper = file_stream_wrapper_get_instance_by_uri('public://');
                        $p = str_replace('public://','',$p);
                        $a = $wrapper->getExternalUrl()."/".$p."/".$x;
                      }else{
                          $a = $x;
                      }
                  }else{
                      $a = $x;
                  }
                  
              }
          }
          $target = variable_get("fz_songbook_musicpath_target",'target="_blank"');
          $title .= '<span class="field field-label sb_musicpath">' . t ( 'Music path' ) . ':</span><a href="' . $x . '" '.$target.'>' . $a . '</a><br/>';          
      }
  }
  return $title;
}

/**
 * show the comment from a row
 *
 * @param string $e
 * @return string
 */
function _songbook_comment(&$row) {
  $ok = false;
  $cstyle = variable_get ( "fz_songbook_comment", "background-color: #444;color: #eee;" );
  $m = array ();
  $row = str_replace("{comment:","{c:",$row);
  $is = preg_match ( "/{c:[^}]*}/simx", $row, $m );
  if ($is > 0) {
    $comment = str_replace ( array ('{c:','}'), "", $m [0] );
    $comment = '<span class="sb_comment" style="' . $cstyle . '">' . $comment . '</span>';
    $row = preg_replace ( "/{c:[^}]*}/simx", $comment, $row );
    $ok = true;
  }  
  return $ok;
}
/**
 * Notes in this row;
 * @param unknown $e
 * @return boolean
 */
function _songbook_hidden($e) {  
  return strpos ( $e, "#" ) === 0;
}
/**
 * Comment area
 *
 * @param unknown $e
 */
function _soc_eoc(&$e) {
  $ssoc = variable_get ( "fz_songbook_soc", "border-left: 1px solid blue; padding-left: 0.5em;");
  $e = str_replace ( array ("{soc}","{eoc}"), 
	                 array ('<div class="sb_soc" style="' . $ssoc . ';">',"</div>"), 
	   $e );
}
/**
 * Tabs area
 *
 * @param unknown $e
 */
function _sot_eot(&$e) {
  $ssot = variable_get ( "fz_songbook_sot", "border-left: 1px solid blue; padding-left: 0.5em; family-name: courier;" );
  $e = str_replace ( "\r\n", "\n", $e );
  $e = str_replace ( "<sot>\n", "<sot>", $e );
  $e = str_replace ( array (
      "{sot}",
      "{eot}"
  ), array (
      '<div class="sb_tab" style="' . $ssot . ';">',
      "</div>"
  ), $e );
}
/**
 * Highligt area
 *
 * @param unknown $e
 */
function _soh_eoh(&$e) {
  $ssoh = variable_get ( "fz_songbook_soh", "background-color: yellow; border-left: 1px solid blue; padding-left: 0.5em;" );
  $is = preg_match ( "/{soh [^}]}/simx", $e, $m );
  $e = str_replace ( array (
      "{soh}",
      "{eoh}"
  ), array (
      '<div class="sb_hi" style="' . $ssoh . ';">',
      "</div>"
  ), $e );
}
/**
 * Chord Grid
 *
 * @param unknown $e
 */
function _sog_eog(&$e) {
  $ssog = variable_get ( "fz_songbook_sog", "background-color: yellow; border-left: 1px solid blue; padding-left: 0.5em;" );
  $is = preg_match ( "/{sog [^}]}/simx", $e, $m );
  $e = str_replace ( array (
      "{sog}",
      "{eog}"
  ), array (
      '<div class="sb_cg" style="' . $ssog . ';">',
      "</div>"
  ), $e );
}


/**
 * Chord definition
 *
 * @param unknown $e
 */
function _songbook_define(&$e) {
}
function _songbook_define_instrument() {
}
function _songbook_metronome() {
}
function _songbook_capo() {
}
function _songbook_pause() {
}
function _songbook_zoom() {
}
function _songbook_instrument(&$e) {
    
}
function _songbook_tuning(&$e){
}
function _songbook_new_page() {
}
