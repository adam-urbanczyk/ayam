#!/usr/bin/perl
# Perl Script to add, height and width tags to all images in a web site.

require "pwd.pl";
require "getopts.pl";

# Need giftopnm (from the netpbm package) to find sizes of the images.
$giftopnm ="/usr/bin/giftopnm";

# Default root url (include trailing slash);

$rooturl = ".";

# Pattern of files to operate on, 

# .html or .shtml or .htm or .tmpl

$htmlpat = "\\.html\$|\\.shtml\$|\\.htm\$|\\.tmpl\$";

# Unix find command (for recusively finding the files in subdirectories to process);

$find = "/usr/bin/find . -print";

# Name of temporary file to use

$tempfile = "/tmp/tempfile";

# Unix mv command  (change this for other OSs)

$move = "mv";

&Getopts('hrfd:u:');

# End of user configurable options
# change anything below this at your own risk

if ($opt_h){
  print "siteheight [-h] [-r] [-d dir] [-u [-f] [files]\n\n",        
        "puts the correct height and width tags into html files\n\n",
        "options: \n\n",
        "   h              -   print this help informaton and exit\n\n",
        "   r              -   recursively decend subdirectories\n\n",
        "   d dir          -   start at directory dir\n\n",
        "   f              -   only process the named files\n\n",
        "   u serverroot   -   please supply the path to the root directory of your website here.\n\n",
        "if called without auguments this script will act on all html files in the current directory\n",
        "it will not find images of form <IMG SRC=\"/pic_dir/yourpic.gif\"> unless you supply the\n",
        "correct serverroot with -u serverroot\n",
        "nor will can does it work on tags containing the full URL http://site_name/pic_dir/yourpic.gif\n",
        "since i didn't want to mess around with libwww.pl\n";
  exit;
}        

&initpwd;

$startdir = $ENV{"PWD"};

if ($opt_u){
  $rooturl= $opt_u;
}

if ($opt_d){
  &chdir($opt_d) || die("Can`t find directory '$opt_d'\n");
}

# Get list files to process

if ($opt_r){
  open (FL,"$find |");
  while(<FL>){
    chop;
    push(@filelist,$_) if /$htmlpat/;
  }
 close(FL);
} elsif ($opt_f){
 @filelist = @ARGV;
} else {
  opendir(FL,'.') || die("Where has your current directory gone\n");
  while($_=readdir(FL)){
    push(@filelist,$_) if /$htmlpat/;
  }
 close(FL);
}

# Now process the named files;

while($file = shift(@filelist)){
#  print "$file\n";
  $dir = '';

  &chdir($startdir);
  if (!open(IN,$file)){
    print STDERR "Couldn`t open $file for input, skipping\n\n";
    next;
  }
  if ($file =~ /\//){
    @path = split(/\//,$file);
    pop(@path);
    $dir = join("/",@path);
    &chdir($startdir);
    &chdir($dir) || die("This pathname '$dir' confuses me\n");
  }  
  
  if (!open(OUT,">$tempfile")){
    die("Fatal: Couldn`t write to temporary file $tempfile
 Please edit the script to set a new tempfile location\n");
  }
  $changes=0;
  while(<IN>){
#    Note how the following search and replace command, calls a subroutine.
#    That the power of perl.
    $changes += s/<IMG\s+SRC\s*=\s*\"([^"]*)\"\s*(.*)>/&procimagetag($1,$2)/ieg;
    print OUT $_;
  }  
  close(IN);
  close(OUT);
  if ($changes){
    &chdir($startdir);
    system("$move $tempfile $file");
  }
}

&chdir($startdir);

# End of main script;

sub procimagetag {
  local($image,$options)=@_;
  local($oldtag);
# handle relative urls;
  $oldtag = "<IMG SRC=\"$image\" $options>";
#  Only works on gifs
  if ($image !~ /\.gif$/){return $oldtag;}
#  Only deal with local files
  if ($image =~ /^http:\\/){return $oldtag;}

#  Remove existing height and width
  $options =~ s/height\s*=\s*\S+//ig;
  $options =~ s/width\s*=\s*\S+//ig;
  
  ($height,$width) = &getheight($image);
  if ($height && $width){
    return ("<IMG SRC=\"$image\" HEIGHT=$height WIDTH=$width $options>");
  } else {
 return ($oldtag);  
  }
}

sub getheight {
  local($image)=@_;
  if ($image=~ /^\//){$image = $rooturl.$image;}
  local($top,$line);
  if ($width{$image}){
    return( $height{$image},$width{$image} );
  }
  open(PPM,"$giftopnm $image |");
  $top = <PPM>;
  if ($top !~ /P\d\n/){
    print STDERR "broken link to image '$image' in file '$file'\n";
    return;
  }
  $line = <PPM>;
  close(PPM);
  ($width,$height)=split(/\s/,$line);
  $width{$image}=$width;
  $height{$image}=$height;
  return($height,$width);
}
