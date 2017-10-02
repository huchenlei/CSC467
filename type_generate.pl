use strict;
use warnings;

my @types = qw [
T_VOID
T_INT
T_BOOL
T_FLOAT
T_VEC2
T_VEC3
T_VEC4
T_BVEC2
T_BVEC3
T_BVEC4
T_IVEC2
T_IVEC3
T_IVEC4
];

foreach my $type (@types) {
    $type =~ /T_([\w\d]+)/;
    print lc($1) . "/{DELIM} {yTRACE($type); return $type;}\n";
}


my @tokens = qw [
IDENTIFIER
COMMA
SEMICOLON
LBRAC
RBRAC
LSBRAC
RSBRAC
LSCOPE
RSCOPE
ADD
SUB
MUL
DIV
POW
ASSIGN
AND
OR
NOT
EQ
NEQ
GT
LT
GE
LE
T_VOID
T_INT
T_BOOL
T_FLOAT
T_VEC2
T_VEC3
T_VEC4
T_BVEC2
T_BVEC3
T_BVEC4
T_IVEC2
T_IVEC3
T_IVEC4
V_INT
V_FLOAT
V_BOOL
IF
ELSE
WHILE
CONST
];

foreach my $token (@tokens) {
    if (not $token =~ /^\/\*.+?/) {
        print "  |     $token\n";
    }
}
