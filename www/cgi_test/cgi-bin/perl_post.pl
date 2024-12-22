local ($buffer, @pairs, $pair, $name, $value, %FORM);

$ENV{'REQUEST_METHOD'} =~ tr/a-z/A-Z/;

if ($ENV{'REQUEST_METHOD'} eq "GET") {
   $buffer = $ENV{'QUERY_STRING'};
}

@pairs = split(/&/, $buffer);

foreach $pair (@pairs) {
   ($name, $value) = split(/=/, $pair);
   $value =~ tr/+/ /;
   $value =~ s/%(..)/pack("C", hex($1))/eg;
   $FORM{$name} = $value;
}

$first_name = $FORM{first_name};
$last_name  = $FORM{last_name};

print "Content-type: text/html\n";
print "<html>";
print "<head>";
print "<title>Hello</title>";
print "</head>";
print "<body>";
print "<h2>Hello $first_name $last_name</h2>";
print "</body>";
print "</html>";

1;
