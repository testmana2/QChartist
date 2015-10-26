const char *my_par_pl =
"eval 'exec /usr/bin/perl  -S $0 ${1+\"$@\"}'\n"
"if 0; # not running under some shell\n"
"package __par_pl;\n"
"my ($par_temp, $progname, @tmpfile);\n"
"END { if ($ENV{PAR_CLEAN}) {\n"
"require File::Temp;\n"
"require File::Basename;\n"
"require File::Spec;\n"
"my $topdir = File::Basename::dirname($par_temp);\n"
"outs(qq{Removing files in \"$par_temp\"});\n"
"File::Find::finddepth(sub { ( -d ) ? rmdir : unlink }, $par_temp);\n"
"rmdir $par_temp;\n"
"if (-d $par_temp && $^O ne 'MSWin32') {\n"
"my $tmp = new File::Temp(\n"
"TEMPLATE => 'tmpXXXXX',\n"
"DIR => File::Basename::dirname($topdir),\n"
"SUFFIX => '.cmd',\n"
"UNLINK => 0,\n"
");\n"
"print $tmp \"#!/bin/sh\n"
"x=1; while [ \\$x -lt 10 ]; do\n"
"rm -rf '$par_temp'\n"
"if [ \\! -d '$par_temp' ]; then\n"
"break\n"
"fi\n"
"sleep 1\n"
"x=`expr \\$x + 1`\n"
"done\n"
"rm '\" . $tmp->filename . \"'\n"
"\";\n"
"chmod 0700,$tmp->filename;\n"
"my $cmd = $tmp->filename . ' >/dev/null 2>&1 &';\n"
"close $tmp;\n"
"system($cmd);\n"
"outs(qq(Spawned background process to perform cleanup: )\n"
". $tmp->filename);\n"
"}\n"
"} }\n"
"BEGIN {\n"
"Internals::PAR::BOOT() if defined &Internals::PAR::BOOT;\n"
"eval {\n"
"_par_init_env();\n"
"if (exists $ENV{PAR_ARGV_0} and $ENV{PAR_ARGV_0} ) {\n"
"@ARGV = map $ENV{\"PAR_ARGV_$_\"}, (1 .. $ENV{PAR_ARGC} - 1);\n"
"$0 = $ENV{PAR_ARGV_0};\n"
"}\n"
"else {\n"
"for (keys %ENV) {\n"
"delete $ENV{$_} if /^PAR_ARGV_/;\n"
"}\n"
"}\n"
"my $quiet = !$ENV{PAR_DEBUG};\n"
"my %Config = (\n"
"path_sep    => ($^O =~ /^MSWin/ ? ';' : ':'),\n"
"_exe        => ($^O =~ /^(?:MSWin|OS2|cygwin)/ ? '.exe' : ''),\n"
"_delim      => ($^O =~ /^MSWin|OS2/ ? '\\\\' : '/'),\n"
");\n"
"_set_progname();\n"
"_set_par_temp();\n"
"my ($start_pos, $data_pos);\n"
"{\n"
"local $SIG{__WARN__} = sub {};\n"
"open _FH, '<', $progname or last;\n"
"binmode(_FH);\n"
"my $buf;\n"
"seek _FH, -8, 2;\n"
"read _FH, $buf, 8;\n"
"last unless $buf eq \"\\nPAR.pm\\n\";\n"
"seek _FH, -12, 2;\n"
"read _FH, $buf, 4;\n"
"seek _FH, -12 - unpack(\"N\", $buf), 2;\n"
"read _FH, $buf, 4;\n"
"$data_pos = (tell _FH) - 4;\n"
"my %require_list;\n"
"while ($buf eq \"FILE\") {\n"
"read _FH, $buf, 4;\n"
"read _FH, $buf, unpack(\"N\", $buf);\n"
"my $fullname = $buf;\n"
"outs(qq(Unpacking file \"$fullname\"...));\n"
"my $crc = ( $fullname =~ s|^([a-f\\d]{8})/|| ) ? $1 : undef;\n"
"my ($basename, $ext) = ($buf =~ m|(?:.*/)?(.*)(\\..*)|);\n"
"read _FH, $buf, 4;\n"
"read _FH, $buf, unpack(\"N\", $buf);\n"
"if (defined($ext) and $ext !~ /\\.(?:pm|pl|ix|al)$/i) {\n"
"my ($out, $filename) = _tempfile($ext, $crc);\n"
"if ($out) {\n"
"binmode($out);\n"
"print $out $buf;\n"
"close $out;\n"
"chmod 0755, $filename;\n"
"}\n"
"$PAR::Heavy::FullCache{$fullname} = $filename;\n"
"$PAR::Heavy::FullCache{$filename} = $fullname;\n"
"}\n"
"elsif ( $fullname =~ m|^/?shlib/| and defined $ENV{PAR_TEMP} ) {\n"
"my $filename = \"$ENV{PAR_TEMP}/$basename$ext\";\n"
"outs(\"SHLIB: $filename\\n\");\n"
"open my $out, '>', $filename or die $!;\n"
"binmode($out);\n"
"print $out $buf;\n"
"close $out;\n"
"}\n"
"else {\n"
"$require_list{$fullname} =\n"
"$PAR::Heavy::ModuleCache{$fullname} = {\n"
"buf => $buf,\n"
"crc => $crc,\n"
"name => $fullname,\n"
"};\n"
"}\n"
"read _FH, $buf, 4;\n"
"}\n"
"local @INC = (sub {\n"
"my ($self, $module) = @_;\n"
"return if ref $module or !$module;\n"
"my $filename = delete $require_list{$module} || do {\n"
"my $key;\n"
"foreach (keys %require_list) {\n"
"next unless /\\Q$module\\E$/;\n"
"$key = $_; last;\n"
"}\n"
"delete $require_list{$key} if defined($key);\n"
"} or return;\n"
"$INC{$module} = \"/loader/$filename/$module\";\n"
"if ($ENV{PAR_CLEAN} and defined(&IO::File::new)) {\n"
"my $fh = IO::File->new_tmpfile or die $!;\n"
"binmode($fh);\n"
"print $fh $filename->{buf};\n"
"seek($fh, 0, 0);\n"
"return $fh;\n"
"}\n"
"else {\n"
"my ($out, $name) = _tempfile('.pm', $filename->{crc});\n"
"if ($out) {\n"
"binmode($out);\n"
"print $out $filename->{buf};\n"
"close $out;\n"
"}\n"
"open my $fh, '<', $name or die $!;\n"
"binmode($fh);\n"
"return $fh;\n"
"}\n"
"die \"Bootstrapping failed: cannot find $module!\\n\";\n"
"}, @INC);\n"
"require XSLoader;\n"
"require PAR::Heavy;\n"
"require Carp::Heavy;\n"
"require Exporter::Heavy;\n"
"PAR::Heavy::_init_dynaloader();\n"
"require IO::File;\n"
"while (my $filename = (sort keys %require_list)[0]) {\n"
"unless ($INC{$filename} or $filename =~ /BSDPAN/) {\n"
"if ($filename =~ /\\.pmc?$/i) {\n"
"require $filename;\n"
"}\n"
"else {\n"
"do $filename unless $filename =~ /sitecustomize\\.pl$/;\n"
"}\n"
"}\n"
"delete $require_list{$filename};\n"
"}\n"
"last unless $buf eq \"PK\\003\\004\";\n"
"$start_pos = (tell _FH) - 4;\n"
"}\n"
"my @par_args;\n"
"my ($out, $bundle, $logfh, $cache_name);\n"
"delete $ENV{PAR_APP_REUSE}; # sanitize (REUSE may be a security problem)\n"
"$quiet = 0 unless $ENV{PAR_DEBUG};\n"
"if (!$start_pos or ($ARGV[0] eq '--par-options' && shift)) {\n"
"my %dist_cmd = qw(\n"
"p   blib_to_par\n"
"i   install_par\n"
"u   uninstall_par\n"
"s   sign_par\n"
"v   verify_par\n"
");\n"
"if (@ARGV and $ARGV[0] eq '--reuse') {\n"
"shift @ARGV;\n"
"$ENV{PAR_APP_REUSE} = shift @ARGV;\n"
"}\n"
"else { # normal parl behaviour\n"
"my @add_to_inc;\n"
"while (@ARGV) {\n"
"$ARGV[0] =~ /^-([AIMOBLbqpiusTv])(.*)/ or last;\n"
"if ($1 eq 'I') {\n"
"push @add_to_inc, $2;\n"
"}\n"
"elsif ($1 eq 'M') {\n"
"eval \"use $2\";\n"
"}\n"
"elsif ($1 eq 'A') {\n"
"unshift @par_args, $2;\n"
"}\n"
"elsif ($1 eq 'O') {\n"
"$out = $2;\n"
"}\n"
"elsif ($1 eq 'b') {\n"
"$bundle = 'site';\n"
"}\n"
"elsif ($1 eq 'B') {\n"
"$bundle = 'all';\n"
"}\n"
"elsif ($1 eq 'q') {\n"
"$quiet = 1;\n"
"}\n"
"elsif ($1 eq 'L') {\n"
"open $logfh, \">>\", $2 or die \"XXX: Cannot open log: $!\";\n"
"}\n"
"elsif ($1 eq 'T') {\n"
"$cache_name = $2;\n"
"}\n"
"shift(@ARGV);\n"
"if (my $cmd = $dist_cmd{$1}) {\n"
"delete $ENV{'PAR_TEMP'};\n"
"init_inc();\n"
"require PAR::Dist;\n"
"&{\"PAR::Dist::$cmd\"}() unless @ARGV;\n"
"&{\"PAR::Dist::$cmd\"}($_) for @ARGV;\n"
"exit;\n"
"}\n"
"}\n"
"unshift @INC, @add_to_inc;\n"
"}\n"
"}\n"
"if ($out) {\n"
"{\n"
"require IO::File;\n"
"require Archive::Zip;\n"
"}\n"
"my $par = shift(@ARGV);\n"
"my $zip;\n"
"if (defined $par) {\n"
"open my $fh, '<', $par or die \"Cannot find '$par': $!\";\n"
"binmode($fh);\n"
"bless($fh, 'IO::File');\n"
"$zip = Archive::Zip->new;\n"
"( $zip->readFromFileHandle($fh, $par) == Archive::Zip::AZ_OK() )\n"
"or die \"Read '$par' error: $!\";\n"
"}\n"
"my %env = do {\n"
"if ($zip and my $meta = $zip->contents('META.yml')) {\n"
"$meta =~ s/.*^par:$//ms;\n"
"$meta =~ s/^\\S.*//ms;\n"
"$meta =~ /^  ([^:]+): (.+)$/mg;\n"
"}\n"
"};\n"
"local $/ = \\4;\n"
"if (defined $par) {\n"
"open PAR, '<', $par or die \"$!: $par\";\n"
"binmode(PAR);\n"
"die \"$par is not a PAR file\" unless <PAR> eq \"PK\\003\\004\";\n"
"}\n"
"CreatePath($out) ;\n"
"my $fh = IO::File->new(\n"
"$out,\n"
"IO::File::O_CREAT() | IO::File::O_WRONLY() | IO::File::O_TRUNC(),\n"
"0777,\n"
") or die $!;\n"
"binmode($fh);\n"
"$/ = (defined $data_pos) ? \\$data_pos : undef;\n"
"seek _FH, 0, 0;\n"
"my $loader = scalar <_FH>;\n"
"if (!$ENV{PAR_VERBATIM} and $loader =~ /^(?:#!|\\@rem)/) {\n"
"require PAR::Filter::PodStrip;\n"
"PAR::Filter::PodStrip->new->apply(\\$loader, $0)\n"
"}\n"
"foreach my $key (sort keys %env) {\n"
"my $val = $env{$key} or next;\n"
"$val = eval $val if $val =~ /^['\"]/;\n"
"my $magic = \"__ENV_PAR_\" . uc($key) . \"__\";\n"
"my $set = \"PAR_\" . uc($key) . \"=$val\";\n"
"$loader =~ s{$magic( +)}{\n"
"$magic . $set . (' ' x (length($1) - length($set)))\n"
"}eg;\n"
"}\n"
"$fh->print($loader);\n"
"$/ = undef;\n"
"if ($bundle) {\n"
"require PAR::Heavy;\n"
"PAR::Heavy::_init_dynaloader();\n"
"init_inc();\n"
"require_modules();\n"
"my @inc = sort {\n"
"length($b) <=> length($a)\n"
"} grep {\n"
"!/BSDPAN/\n"
"} grep {\n"
"($bundle ne 'site') or\n"
"($_ ne $Config::Config{archlibexp} and\n"
"$_ ne $Config::Config{privlibexp});\n"
"} @INC;\n"
"my %files;\n"
"/^_<(.+)$/ and -e $1 and $files{$1}++ for keys %::;\n"
"$files{$_}++ for values %INC;\n"
"my $lib_ext = $Config::Config{lib_ext};\n"
"my %written;\n"
"foreach (sort keys %files) {\n"
"my ($name, $file);\n"
"foreach my $dir (@inc) {\n"
"if ($name = $PAR::Heavy::FullCache{$_}) {\n"
"$file = $_;\n"
"last;\n"
"}\n"
"elsif (/^(\\Q$dir\\E\\/(.*[^Cc]))\\Z/i) {\n"
"($file, $name) = ($1, $2);\n"
"last;\n"
"}\n"
"elsif (m!^/loader/[^/]+/(.*[^Cc])\\Z!) {\n"
"if (my $ref = $PAR::Heavy::ModuleCache{$1}) {\n"
"($file, $name) = ($ref, $1);\n"
"last;\n"
"}\n"
"elsif (-f \"$dir/$1\") {\n"
"($file, $name) = (\"$dir/$1\", $1);\n"
"last;\n"
"}\n"
"}\n"
"}\n"
"next unless defined $name and not $written{$name}++;\n"
"next if !ref($file) and $file =~ /\\.\\Q$lib_ext\\E$/;\n"
"outs( join \"\",\n"
"qq(Packing \"), ref $file ? $file->{name} : $file,\n"
"qq(\"...)\n"
");\n"
"my $content;\n"
"if (ref($file)) {\n"
"$content = $file->{buf};\n"
"}\n"
"else {\n"
"open FILE, '<', $file or die \"Can't open $file: $!\";\n"
"binmode(FILE);\n"
"$content = <FILE>;\n"
"close FILE;\n"
"PAR::Filter::PodStrip->new->apply(\\$content, $file)\n"
"if !$ENV{PAR_VERBATIM} and $name =~ /\\.(?:pm|ix|al)$/i;\n"
"PAR::Filter::PatchContent->new->apply(\\$content, $file, $name);\n"
"}\n"
"outs(qq(Written as \"$name\"));\n"
"$fh->print(\"FILE\");\n"
"$fh->print(pack('N', length($name) + 9));\n"
"$fh->print(sprintf(\n"
"\"%08x/%s\", Archive::Zip::computeCRC32($content), $name\n"
"));\n"
"$fh->print(pack('N', length($content)));\n"
"$fh->print($content);\n"
"}\n"
"}\n"
"$zip->writeToFileHandle($fh) if $zip;\n"
"$cache_name = substr $cache_name, 0, 40;\n"
"if (!$cache_name and my $mtime = (stat($out))[9]) {\n"
"my $ctx = eval { require Digest::SHA; Digest::SHA->new(1) }\n"
"|| eval { require Digest::SHA1; Digest::SHA1->new }\n"
"|| eval { require Digest::MD5; Digest::MD5->new };\n"
"my $sha_version = eval { $Digest::SHA::VERSION } || 0;\n"
"if ($sha_version eq '5.38' or $sha_version eq '5.39') {\n"
"$ctx->addfile($out, \"b\") if ($ctx);\n"
"}\n"
"else {\n"
"if ($ctx and open(my $fh, \"<$out\")) {\n"
"binmode($fh);\n"
"$ctx->addfile($fh);\n"
"close($fh);\n"
"}\n"
"}\n"
"$cache_name = $ctx ? $ctx->hexdigest : $mtime;\n"
"}\n"
"$cache_name .= \"\\0\" x (41 - length $cache_name);\n"
"$cache_name .= \"CACHE\";\n"
"$fh->print($cache_name);\n"
"$fh->print(pack('N', $fh->tell - length($loader)));\n"
"$fh->print(\"\\nPAR.pm\\n\");\n"
"$fh->close;\n"
"chmod 0755, $out;\n"
"exit;\n"
"}\n"
"{\n"
"last unless defined $start_pos;\n"
"_fix_progname();\n"
"require PAR;\n"
"PAR::Heavy::_init_dynaloader();\n"
"{\n"
"require File::Find;\n"
"require Archive::Zip;\n"
"}\n"
"my $zip = Archive::Zip->new;\n"
"my $fh = IO::File->new;\n"
"$fh->fdopen(fileno(_FH), 'r') or die \"$!: $@\";\n"
"$zip->readFromFileHandle($fh, $progname) == Archive::Zip::AZ_OK() or die \"$!: $@\";\n"
"push @PAR::LibCache, $zip;\n"
"$PAR::LibCache{$progname} = $zip;\n"
"$quiet = !$ENV{PAR_DEBUG};\n"
"outs(qq(\\$ENV{PAR_TEMP} = \"$ENV{PAR_TEMP}\"));\n"
"if (defined $ENV{PAR_TEMP}) { # should be set at this point!\n"
"foreach my $member ( $zip->members ) {\n"
"next if $member->isDirectory;\n"
"my $member_name = $member->fileName;\n"
"next unless $member_name =~ m{\n"
"^\n"
"/?shlib/\n"
"(?:$Config::Config{version}/)?\n"
"(?:$Config::Config{archname}/)?\n"
"([^/]+)\n"
"$\n"
"}x;\n"
"my $extract_name = $1;\n"
"my $dest_name = File::Spec->catfile($ENV{PAR_TEMP}, $extract_name);\n"
"if (-f $dest_name && -s _ == $member->uncompressedSize()) {\n"
"outs(qq(Skipping \"$member_name\" since it already exists at \"$dest_name\"));\n"
"} else {\n"
"outs(qq(Extracting \"$member_name\" to \"$dest_name\"));\n"
"$member->extractToFileNamed($dest_name);\n"
"chmod(0555, $dest_name) if $^O eq \"hpux\";\n"
"}\n"
"}\n"
"}\n"
"}\n"
"unless ($PAR::LibCache{$progname}) {\n"
"die << \".\" unless @ARGV;\n"
"Usage: $0 [ -Alib.par ] [ -Idir ] [ -Mmodule ] [ src.par ] [ program.pl ]\n"
"$0 [ -B|-b ] [-Ooutfile] src.par\n"
".\n"
"$ENV{PAR_PROGNAME} = $progname = $0 = shift(@ARGV);\n"
"}\n"
"sub CreatePath {\n"
"my ($name) = @_;\n"
"require File::Basename;\n"
"my ($basename, $path, $ext) = File::Basename::fileparse($name, ('\\..*'));\n"
"require File::Path;\n"
"File::Path::mkpath($path) unless(-e $path); # mkpath dies with error\n"
"}\n"
"sub require_modules {\n"
"require lib;\n"
"require DynaLoader;\n"
"require integer;\n"
"require strict;\n"
"require warnings;\n"
"require vars;\n"
"require Carp;\n"
"require Carp::Heavy;\n"
"require Errno;\n"
"require Exporter::Heavy;\n"
"require Exporter;\n"
"require Fcntl;\n"
"require File::Temp;\n"
"require File::Spec;\n"
"require XSLoader;\n"
"require Config;\n"
"require IO::Handle;\n"
"require IO::File;\n"
"require Compress::Zlib;\n"
"require Archive::Zip;\n"
"require PAR;\n"
"require PAR::Heavy;\n"
"require PAR::Dist;\n"
"require PAR::Filter::PodStrip;\n"
"require PAR::Filter::PatchContent;\n"
"require attributes;\n"
"eval { require Cwd };\n"
"eval { require Win32 };\n"
"eval { require Scalar::Util };\n"
"eval { require Archive::Unzip::Burst };\n"
"eval { require Tie::Hash::NamedCapture };\n"
"eval { require PerlIO; require PerlIO::scalar };\n"
"}\n"
"sub _set_par_temp {\n"
"if (defined $ENV{PAR_TEMP} and $ENV{PAR_TEMP} =~ /(.+)/) {\n"
"$par_temp = $1;\n"
"return;\n"
"}\n"
"foreach my $path (\n"
"(map $ENV{$_}, qw( PAR_TMPDIR TMPDIR TEMPDIR TEMP TMP )),\n"
"qw( C:\\\\TEMP /tmp . )\n"
") {\n"
"next unless defined $path and -d $path and -w $path;\n"
"my $username;\n"
"my $pwuid;\n"
"eval {($pwuid) = getpwuid($>) if defined $>;};\n"
"if ( defined(&Win32::LoginName) ) {\n"
"$username = &Win32::LoginName;\n"
"}\n"
"elsif (defined $pwuid) {\n"
"$username = $pwuid;\n"
"}\n"
"else {\n"
"$username = $ENV{USERNAME} || $ENV{USER} || 'SYSTEM';\n"
"}\n"
"$username =~ s/\\W/_/g;\n"
"my $stmpdir = \"$path$Config{_delim}par-\".unpack(\"H*\", $username);\n"
"mkdir $stmpdir, 0755;\n"
"if (!$ENV{PAR_CLEAN} and my $mtime = (stat($progname))[9]) {\n"
"open (my $fh, \"<\". $progname);\n"
"seek $fh, -18, 2;\n"
"sysread $fh, my $buf, 6;\n"
"if ($buf eq \"\\0CACHE\") {\n"
"seek $fh, -58, 2;\n"
"sysread $fh, $buf, 41;\n"
"$buf =~ s/\\0//g;\n"
"$stmpdir .= \"$Config{_delim}cache-\" . $buf;\n"
"}\n"
"else {\n"
"my $ctx = eval { require Digest::SHA; Digest::SHA->new(1) }\n"
"|| eval { require Digest::SHA1; Digest::SHA1->new }\n"
"|| eval { require Digest::MD5; Digest::MD5->new };\n"
"my $sha_version = eval { $Digest::SHA::VERSION } || 0;\n"
"if ($sha_version eq '5.38' or $sha_version eq '5.39') {\n"
"$ctx->addfile($progname, \"b\") if ($ctx);\n"
"}\n"
"else {\n"
"if ($ctx and open(my $fh, \"<$progname\")) {\n"
"binmode($fh);\n"
"$ctx->addfile($fh);\n"
"close($fh);\n"
"}\n"
"}\n"
"$stmpdir .= \"$Config{_delim}cache-\" . ( $ctx ? $ctx->hexdigest : $mtime );\n"
"}\n"
"close($fh);\n"
"}\n"
"else {\n"
"$ENV{PAR_CLEAN} = 1;\n"
"$stmpdir .= \"$Config{_delim}temp-$$\";\n"
"}\n"
"$ENV{PAR_TEMP} = $stmpdir;\n"
"mkdir $stmpdir, 0755;\n"
"last;\n"
"}\n"
"$par_temp = $1 if $ENV{PAR_TEMP} and $ENV{PAR_TEMP} =~ /(.+)/;\n"
"}\n"
"sub _tempfile {\n"
"my ($ext, $crc) = @_;\n"
"my ($fh, $filename);\n"
"$filename = \"$par_temp/$crc$ext\";\n"
"if ($ENV{PAR_CLEAN}) {\n"
"unlink $filename if -e $filename;\n"
"push @tmpfile, $filename;\n"
"}\n"
"else {\n"
"return (undef, $filename) if (-r $filename);\n"
"}\n"
"open $fh, '>', $filename or die $!;\n"
"binmode($fh);\n"
"return($fh, $filename);\n"
"}\n"
"sub _set_progname {\n"
"if (defined $ENV{PAR_PROGNAME} and $ENV{PAR_PROGNAME} =~ /(.+)/) {\n"
"$progname = $1;\n"
"}\n"
"$progname ||= $0;\n"
"if ($ENV{PAR_TEMP} and index($progname, $ENV{PAR_TEMP}) >= 0) {\n"
"$progname = substr($progname, rindex($progname, $Config{_delim}) + 1);\n"
"}\n"
"if (!$ENV{PAR_PROGNAME} or index($progname, $Config{_delim}) >= 0) {\n"
"if (open my $fh, '<', $progname) {\n"
"return if -s $fh;\n"
"}\n"
"if (-s \"$progname$Config{_exe}\") {\n"
"$progname .= $Config{_exe};\n"
"return;\n"
"}\n"
"}\n"
"foreach my $dir (split /\\Q$Config{path_sep}\\E/, $ENV{PATH}) {\n"
"next if exists $ENV{PAR_TEMP} and $dir eq $ENV{PAR_TEMP};\n"
"$dir =~ s/\\Q$Config{_delim}\\E$//;\n"
"(($progname = \"$dir$Config{_delim}$progname$Config{_exe}\"), last)\n"
"if -s \"$dir$Config{_delim}$progname$Config{_exe}\";\n"
"(($progname = \"$dir$Config{_delim}$progname\"), last)\n"
"if -s \"$dir$Config{_delim}$progname\";\n"
"}\n"
"}\n"
"sub _fix_progname {\n"
"$0 = $progname ||= $ENV{PAR_PROGNAME};\n"
"if (index($progname, $Config{_delim}) < 0) {\n"
"$progname = \".$Config{_delim}$progname\";\n"
"}\n"
"my $pwd = (defined &Cwd::getcwd) ? Cwd::getcwd()\n"
": ((defined &Win32::GetCwd) ? Win32::GetCwd() : `pwd`);\n"
"chomp($pwd);\n"
"$progname =~ s/^(?=\\.\\.?\\Q$Config{_delim}\\E)/$pwd$Config{_delim}/;\n"
"$ENV{PAR_PROGNAME} = $progname;\n"
"}\n"
"sub _par_init_env {\n"
"if ( $ENV{PAR_INITIALIZED}++ == 1 ) {\n"
"return;\n"
"} else {\n"
"$ENV{PAR_INITIALIZED} = 2;\n"
"}\n"
"for (qw( SPAWNED TEMP CLEAN DEBUG CACHE PROGNAME ARGC ARGV_0 ) ) {\n"
"delete $ENV{'PAR_'.$_};\n"
"}\n"
"for (qw/ TMPDIR TEMP CLEAN DEBUG /) {\n"
"$ENV{'PAR_'.$_} = $ENV{'PAR_GLOBAL_'.$_} if exists $ENV{'PAR_GLOBAL_'.$_};\n"
"}\n"
"my $par_clean = \"__ENV_PAR_CLEAN__               \";\n"
"if ($ENV{PAR_TEMP}) {\n"
"delete $ENV{PAR_CLEAN};\n"
"}\n"
"elsif (!exists $ENV{PAR_GLOBAL_CLEAN}) {\n"
"my $value = substr($par_clean, 12 + length(\"CLEAN\"));\n"
"$ENV{PAR_CLEAN} = $1 if $value =~ /^PAR_CLEAN=(\\S+)/;\n"
"}\n"
"}\n"
"sub outs {\n"
"return if $quiet;\n"
"if ($logfh) {\n"
"print $logfh \"@_\\n\";\n"
"}\n"
"else {\n"
"print \"@_\\n\";\n"
"}\n"
"}\n"
"sub init_inc {\n"
"require Config;\n"
"push @INC, grep defined, map $Config::Config{$_}, qw(\n"
"archlibexp privlibexp sitearchexp sitelibexp\n"
"vendorarchexp vendorlibexp\n"
");\n"
"}\n"
"package main;\n"
"require PAR;\n"
"unshift @INC, \\&PAR::find_par;\n"
"PAR->import(@par_args);\n"
"die qq(par.pl: Can't open perl script \"$progname\": No such file or directory\\n)\n"
"unless -e $progname;\n"
"do $progname;\n"
"CORE::exit($1) if ($@ =~/^_TK_EXIT_\\((\\d+)\\)/);\n"
"die $@ if $@;\n"
"};\n"
"$::__ERROR = $@ if $@;\n"
"}\n"
"CORE::exit($1) if ($::__ERROR =~/^_TK_EXIT_\\((\\d+)\\)/);\n"
"die $::__ERROR if $::__ERROR;\n"
"1;\n"
"__END__\n"
;