import ply.lex as lex
import ply.yacc as yacc
from functools import reduce
from itertools import chain, combinations

def powerset(iterable):
    s = list(iterable)
    return chain.from_iterable(combinations(s, r) for r in range(len(s)+1))

# List of token names.   This is always required
tokens = (
  'SPACE',
  'FF',
  'LPAREN',
  'RPAREN',
  'SHARE',
  'RANDOM'
)

ids = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
assert len(ids) == 10 + 26 + 26

def MyParser(d, names_r, glitch=False):

    nb_r = len(names_r)
    ## LEXER

    # Regular expression rules for simple tokens
    t_SPACE   = r'\s+'
    t_FF      = r'\|'
    t_LPAREN  = r'\('
    t_RPAREN  = r'\)'

    precedence = (
            ('left', 'FF'),
            ('left', 'SPACE')
            )

    # A regular expression rule with some action code
    def t_SHARE(t):
        r's[0-9a-zA-Z]{2}'

        probe_r = vector(GF(2), nb_r)
        probe_r.set_immutable()

        probe_sh = matrix(GF(2), d + 1)
        i = ids.index(t.value[1])
        j = ids.index(t.value[2])
        probe_sh[i, j] = 1
        probe_sh.set_immutable()

        s = set()
        s.add((probe_r, probe_sh))

        t.value = [s, t.value, set()]
        return t

    def t_RANDOM(t):
        r'r[0-9a-zA-Z]+'

        if t.value not in names_r:
            raise ValueError(t.value + " should be declared first")

        probe_r = vector(GF(2), nb_r)
        i = names_r.index(t.value)
        probe_r[i] = 1
        probe_r.set_immutable()

        probe_sh = matrix(GF(2), d + 1)
        probe_sh.set_immutable()

        s = set()
        s.add((probe_r, probe_sh))

        t.value = [s, t.value, set()]
        return t

    # Error handling rule
    def t_error(t):
        print("Token not recognised: {}".format(t.value))
        raise ValueError

    # Build the lexer from my environment and return it    
    lexer = lex.lex()

    ## PARSER 

    def p_expr_imm(t):
        '''expr : SHARE
                | RANDOM
        '''
        t[0] = t[1]

    def p_expr_paren(t):
        'expr : LPAREN expr RPAREN'
        t[0] = [t[2][0], '(' + t[2][1] + ')', t[2][2]]

    def p_expr_xor(t):
        'expr : expr SPACE expr'

        probe_expl = t[1][1] + ' ' + t[3][1]
        probes = set()

        if glitch:
            s = t[1][0].union(t[3][0])

            for e in powerset(s):
                if len(e) == 0:
                    continue
                probe_r = vector(GF(2), nb_r)
                probe_sh = matrix(GF(2), d + 1)
                for l in e:
                    probe_r += l[0]
                    probe_sh += l[1]

                probe_r.set_immutable()
                probe_sh.set_immutable()
                probes.add((probe_r, probe_sh, probe_expl))

        else:
            assert len(t[1][0]) == 1 and len(t[3][0]) == 1
            probe_r1, probe_sh1 = t[1][0].pop()
            probe_r2, probe_sh2 = t[3][0].pop()

            probe_r = probe_r1 + probe_r2
            probe_r.set_immutable()
            probe_sh = probe_sh1 + probe_sh2
            probe_sh.set_immutable()

            probes.add((probe_r, probe_sh, probe_expl))
            
            s = set()
            s.add((probe_r, probe_sh))

        new_probes = t[1][2].union(t[3][2]).union(probes)
        t[0] = [s, probe_expl, new_probes]

    def p_expr_ff(t):
        '''expr : expr SPACE FF
                | expr FF'''

        s = set()
        new_p = reduce(lambda x, y: (x[0]+y[0],x[1]+y[1]), t[1][0])
        new_p[0].set_immutable()
        new_p[1].set_immutable()
        s.add(new_p)

        if t[2] == '|':
            probe_expl = t[1][1] + '|'
        else:
            probe_expl = t[1][1] + ' |'
        t[0] = [s, probe_expl, t[1][2]]

    
    def p_error(p):
        if p is None:
            ValueError("Unexpected EOL, check your last char")
        raise ValueError("Wrong Token " + p.type) 


    return yacc.yacc()


