#!/usr/bin/perl
#
# Splits the list of files and outputs for makefile type files 
# wrapped at 80 chars 
# 
# Tom St Denis
@a = split(" ", $ARGV[1]);
$b = "$ARGV[0]=";
$len = length($b);
print $b;
foreach my $obj (@a) {
   $len = $len + length($obj);
   $obj =~ s/\*/\$/;
   if ($len > 100) {
      printf "\\\n";
      $len = length($obj);
   }
   print "$obj ";
}
if ($ARGV[0] eq "HEADERS") { print "testprof/tomcrypt_test.h"; }

print "\n\n";

# $Source: /usr/local/dslrepos/uClinux-dist/user/dropbear-0.48.1/libtomcrypt/parsenames.pl,v $   
# $Revision: 1.1 $   
# $Date: 2006/06/08 13:36:01 $ 
