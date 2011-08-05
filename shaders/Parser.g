{-# OPTIONS -w #-}
module Parser (parseShader) where
import Data.Char (isDigit)
import Data.List (intersperse)
import Control.Monad.Error (ErrorT, runErrorT)
import Control.Monad.Identity (Identity, runIdentity)
import Control.Monad.State (StateT, runStateT)

import Lex
import Shader

%{

Terminal = TKps10               as "ps.1.0"
         | TKps11               as "ps.1.1"
         | TKps12               as "ps.1.2"
         | TKps13               as "ps.1.3"
         | TKps14               as "ps.1.4"
         | TKps20               as "ps.2.0"
         | TKvs11               as "vs.1.1"

         | TKabs                as "abs"
         | TKadd                as "add"
         | TKcmp                as "cmp"
         | TKdcl                as "dcl"
         | TKdcl2d              as "dcl_2d"
         | TKdclcolor           as "dcl_color"
         | TKdclnormal          as "dcl_normal"
         | TKdclposition        as "dcl_position"
         | TKdcltexcoord        as "dcl_texcoord"
         | TKdef                as "def"
         | TKdp3                as "dp3"
         | TKdp4                as "dp4"
         | TKlrp                as "lrp"
         | TKm3x3               as "m3x3"
         | TKm4x3               as "m4x3"
         | TKm4x4               as "m4x4"
         | TKmad                as "mad"
         | TKmax                as "max"
         | TKmin                as "min"
         | TKmov                as "mov"
         | TKmul                as "mul"
         | TKrcp                as "rcp"
         | TKrsq                as "rsq"
         | TKsub                as "sub"
         | TKtex                as "tex"
         | TKtexbem             as "texbem"
         | TKtexcrd             as "texcrd"
         | TKtexkill            as "texkill"
         | TKtexld              as "texld"
         | TKtexldp             as "texldp"
         
         | TKaddrreg {String}    as "address register"
         | TKconstreg {String}   as "constant register"
         | TKoutreg {String }    as "output register"
         | TKsamplerreg {String} as "sampler register"
         | TKtempreg {String}    as "temporary register"
         | TKtexreg {String}     as "texture register"
         | TKvertreg {String}    as "vertex register"
         
         | TKselector {String}  as "selector"
         | TKdwxyw              as "_dw.xyw"
         | TKinstSat            as "_SAT"
         | TKinstX2             as "_X2"
         | TKinstD8             as "_D8"
         
         | TKcomment {String}   as ";"
         | TKcomma              as ","
         | TKfloat {String}     as "floating point number"
         | TKint {Int}          as "integer"
         | TKplus               as "+"
         | TKminus              as "-"
         | TKobracket           as "["
         | TKcbracket           as "]"

         | *TKEOF               as "<end of input>";


shader { Shader };
shader { PixelShader s  } : "ps.1.0", stmts {s};
       { PixelShader s  } | "ps.1.1", stmts {s};
       { PixelShader s  } | "ps.1.2", stmts {s};
       { PixelShader s  } | "ps.1.3", stmts {s};
       { PixelShader s  } | "ps.1.4", stmts {s};
       { PixelShader s  } | "ps.2.0", stmts {s};
       { VertexShader s } | "vs.1.1", stmts {s};


stmts { [Stmt] };
stmts { s } : many stmt {s};

stmt { Stmt };
stmt { Abs dst src }            : "abs", reg {dst}, ",", reg {src};
     { Add dst src0 src1 }      | "add", reg {dst}, ",", reg {src0}, ",", reg {src1};
     { AddSat dst src0 src1 }   | "add", "_SAT", reg {dst}, ",", reg {src0}, ",", reg {src1};
     { Cmp dst src0 src1 src2 } | "cmp", reg {dst}, ",", reg {src0}, ",", reg {src1}, ",", reg {src2};
     { Cmp dst src0 src1 src2 } | "+", "cmp", reg {dst}, ",", reg {src0}, ",", reg {src1}, ",", reg {src2};
     { Dcl dst }                | "dcl", reg {dst};
     { Dcl2D dst }              | "dcl_2d", reg {dst};
     { DclColor dst }           | "dcl_color", reg {dst};
     { DclNormal dst }          | "dcl_normal", reg {dst};
     { DclPosition dst }        | "dcl_position", reg {dst};
     { DclTexcoord n dst }      | "dcl_texcoord", TKint {n}, reg {dst};
     { Def dst x y z w }        | "def", reg {dst}, ",", float {x}, ",", float {y}, ",", float {z}, ",", float {w};
     { Dp3 dst src0 src1 }      | "dp3", reg {dst}, ",", reg {src0}, ",", reg {src1};
     { Dp4 dst src0 src1 }      | "dp4", reg {dst}, ",", reg {src0}, ",", reg {src1};
     { Lrp dst src0 src1 src2 } | "lrp", reg {dst}, ",", reg {src0}, ",", reg {src1}, ",", reg {src2};
     { M3x3 dst src0 src1 }     | "m3x3", reg {dst}, ",", reg {src0}, ",", reg {src1};
     { M4x3 dst src0 src1 }     | "m4x3", reg {dst}, ",", reg {src0}, ",", reg {src1};
     { M4x4 dst src0 src1 }     | "m4x4", reg {dst}, ",", reg {src0}, ",", reg {src1};
     { Mad dst src0 src1 src2 } | "mad", reg {dst}, ",", reg {src0}, ",", reg {src1}, ",", reg {src2};
     { Mad dst src0 src1 src2 } | "+", "mad", reg {dst}, ",", reg {src0}, ",", reg {src1}, ",", reg {src2};
     { MadSat dst src0 src1 src2 } | "mad", "_SAT",  reg {dst}, ",", reg {src0}, ",", reg {src1}, ",", reg {src2};
     { Max dst src0 src1 }      | "max", reg {dst}, ",", reg {src0}, ",", reg {src1};
     { Min dst src0 src1 }      | "min", reg {dst}, ",", reg {src0}, ",", reg {src1};
     { Mov dst src }            | "mov", reg {dst}, ",", reg {src};
     { Mov dst src }            | "+", "mov", reg {dst}, ",", reg {src};
     { MovD8 dst src }          | "mov", "_D8", reg {dst}, ",", reg {src};
     { Mul dst src0 src1 }      | "mul", reg {dst}, ",", reg {src0}, ",", reg {src1};
     { Mul dst src0 src1 }      | "+", "mul", reg {dst}, ",", reg {src0}, ",", reg {src1};
     { Mul2X dst src0 src1 }    | "mul", "_X2", reg {dst}, ",", reg {src0}, ",", reg {src1};
     { Mul2XSat dst src0 src1 } | "mul", "_X2", "_SAT", reg {dst}, ",", reg {src0}, ",", reg {src1};
     { Rcp dst src }            | "rcp", reg {dst}, ",", reg {src};
     { Rsq dst src }            | "rsq", reg {dst}, ",", reg {src};
     { Sub dst src0 src1 }      | "sub", reg {dst}, ",", reg {src0}, ",", reg {src1};
     { Sub dst src0 src1 }      | "+", "sub", reg {dst}, ",", reg {src0}, ",", reg {src1};
     { SubSat dst src0 src1 }   | "sub", "_SAT", reg {dst}, ",", reg {src0}, ",", reg {src1};
     { Tex r }                  | "tex", reg {r};
     { Texbem dst src }         | "texbem", reg {dst}, ",", reg {src};
     { Texcrd dst src }         | "texcrd", reg {dst}, ",", reg {src};
     { Texkill r }              | "texkill", reg {r};
     { Texld dst src (sR src) } | "texld", reg {dst}, ",", reg {src};
     { Texld dst src0 src1 }    | "texld", reg {dst}, ",", reg {src0}, ",", reg {src1};
     { Texldp dst src0 src1 }   | "texldp", reg {dst}, ",", reg {src0}, ",", reg {src1};
     { TexldDiv dst src (sR src) } | "texld", reg {dst}, ",", reg {src}, "_dw.xyw";
     { Comment str}             | ";" {str};


float { String };
float { show n } : TKint {n};
      { f }      | TKfloat {f};


basereg { String };
basereg { r } : TKaddrreg {r};
        { r } | TKconstreg {r};
        { r } | TKoutreg {r};
        { r } | TKsamplerreg {r};
        { r } | TKtempreg {r};
        { r } | TKtexreg {r};
        { r } | TKvertreg {r};


reg { Reg };
reg { BaseReg r }           : basereg {r};
    { RegSel r s }          | basereg {r}, TKselector {s};
    { RegAddr r (a ++ x) 0 }       | basereg {r}, "[", basereg {a}, TKselector {x}, "]";
    { RegAddr r "" i }      | basereg {r}, "[", TKint {i}, "]";
    { RegAddrSel r (a ++ x) 0 s }  | basereg {r}, "[", basereg {a}, TKselector {x}, "]", TKselector {s};
    { RegAddrSel r "" i s } | basereg {r}, "[", TKint {i}, "]", TKselector {s};
    { OneMinus r }          | TKint {1}, "-", basereg {r};
    { Minus r s }           | "-", basereg {r}, TKselector {s};
    { Minus r "" }          | "-", basereg {r};


{-
    {  } | ;
reg { Reg r "-" s }  : "-", TKtempreg {r}, TKselector {s};
    { Reg r "-" "" } | "-", TKtempreg {r};
    { Reg r "" "" }  | TKconstreg {r};
    { Reg r "" "" }  | TKoutreg {r};
    { Reg r "" "" }  | TKtempreg {r};
    { Reg r "" "" }  | TKtexreg {r};
    { Reg r "" "" }  | TKvertreg {r};
    { reg }          | samplerreg {reg}; 
    { Reg r s "" }   | TKconstreg {r}, TKselector {s};
    { Reg r s "" }   | TKoutreg {r}, TKselector {s};
    { Reg r s "" }   | TKtempreg {r}, TKselector {s};
    { Reg r s "" }   | TKtexreg {r}, TKselector {s};
    { Reg r s "" }   | TKvertreg {r}, TKselector {s};
    { Reg r s s2 }   | TKconstreg {r}, TKselector {s}, TKselector {s2};
    { Reg r s s2 }   | TKtexreg {r}, TKselector {s}, TKselector {s2};
    { Reg r "1-" ""} | TKint {1}, "-", TKtempreg {r};
    { Reg r "1-" ""} | TKint {1}, "-", TKvertreg {r};

-}


}%


sR :: Reg -> Reg
sR (BaseReg base) = BaseReg ('s':(dropWhile (not . isDigit) base))
sR r = error $ "sR: bad reg " ++ (show r)


parseShader :: String -> Either String Shader
parseShader str =
  case runIdentity (runErrorT (runStateT shader (mkInput str))) of
    Left err     -> Left err
    Right (s, _) -> Right s


frown :: [String] -> Terminal -> P a
frown la t = do
  failP $ "\ngot: " ++ (show t)
       ++ "\n* expected: " ++ concat (intersperse ", " la)
