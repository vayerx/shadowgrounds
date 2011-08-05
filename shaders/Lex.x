{
{-# OPTIONS -w #-}
module Lex(mkInput, Terminal(..), P, failP, get) where
import Control.Monad.Error (ErrorT, throwError)
import Control.Monad.Identity (Identity)
import Control.Monad.State (StateT)
import qualified Control.Monad.State as S (get, put)
import Data.Char (isDigit)
import Control.Monad.Trans (lift)

}

$digit = 0-9
$sel = [rgbaxyzw]
$regsel = [abzw]

tokens :-

  $white+             ;
  "ps.1.0"            { tkConst TKps10 }
  "ps.1.1"            { tkConst TKps11 }
  "ps.1.2"            { tkConst TKps12 }
  "ps.1.3"            { tkConst TKps13 }
  "ps.1.4"            { tkConst TKps14 }
  "ps.2.0"            { tkConst TKps20 }
  "vs.1.1"            { tkConst TKvs11 }
  
  "abs"               { tkConst TKabs }
  "add"               { tkConst TKadd }
  "cmp"               { tkConst TKcmp }
  "dcl"               { tkConst TKdcl }
  "dcl_2d"            { tkConst TKdcl2d }
  "dcl_color"         { tkConst TKdclcolor }
  "dcl_normal"        { tkConst TKdclnormal }
  "dcl_position"      { tkConst TKdclposition }
  "dcl_texcoord"      { tkConst TKdcltexcoord }
  "def"               { tkConst TKdef }
  "dp3"               { tkConst TKdp3 }
  "dp4"               { tkConst TKdp4 }
  "lrp"               { tkConst TKlrp }
  "m3x3"              { tkConst TKm3x3 }
  "m4x3"              { tkConst TKm4x3 }
  "m4x4"              { tkConst TKm4x4 }
  "mad"               { tkConst TKmad }
  "max"               { tkConst TKmax }
  "min"               { tkConst TKmin }
  "mov"               { tkConst TKmov }
  "mul"               { tkConst TKmul }
  "rcp"               { tkConst TKrcp }
  "rsq"               { tkConst TKrsq }
  "sub"               { tkConst TKsub }
  "tex"               { tkConst TKtex }
  "texbem"            { tkConst TKtexbem }
  "texcrd"            { tkConst TKtexcrd }
  "texkill"           { tkConst TKtexkill }
  "texld"             { tkConst TKtexld }
  "texldp"            { tkConst TKtexldp }

  ";"                 { comment }
  ","                 { tkConst TKcomma }
  "+"                 { tkConst TKplus }
  "-"                 { tkConst TKminus }
  "["                 { tkConst TKobracket }
  "]"                 { tkConst TKcbracket }
  
  $digit+             { tkValue (TKint . read) }
  $digit?\.$digit+f?  { tkValue (TKfloat . takeWhile (\x -> isDigit x || x == '.')) }
  a$digit             { tkValue TKaddrreg }
  c$digit+            { tkValue TKconstreg }
  c[^ \,.]*           { tkValue TKconstreg }
  o[A-Za-z0-9]*       { tkValue TKoutreg }
  r$digit             { tkValue TKtempreg }
  s$digit             { tkValue TKsamplerreg }
  t$digit             { tkValue TKtexreg }
  v$digit             { tkValue TKvertreg }
  \.$sel+             { tkValue TKselector }
  "_dw.xyw"           { tkConst TKdwxyw }
  
  "_pp"               ;
  "_sat"              { tkConst TKinstSat}
  "_x2"               { tkConst TKinstX2}
  "_d8"               { tkConst TKinstD8}

{


data Terminal
 = TKps10
 | TKps11
 | TKps12
 | TKps13
 | TKps14
 | TKps20
 | TKvs11
 
 | TKabs
 | TKadd
 | TKcmp
 | TKdcl
 | TKdcl2d
 | TKdclcolor
 | TKdclnormal
 | TKdclposition
 | TKdcltexcoord
 | TKdef
 | TKdp3
 | TKdp4
 | TKlrp
 | TKm3x3
 | TKm4x3
 | TKm4x4
 | TKmad
 | TKmax
 | TKmin
 | TKmov
 | TKmul
 | TKrcp
 | TKrsq
 | TKsub
 | TKtex
 | TKtexbem
 | TKtexcrd
 | TKtexkill
 | TKtexld
 | TKtexldp
 
 | TKaddrreg String
 | TKconstreg String
 | TKoutreg String
 | TKsamplerreg String
 | TKtempreg String
 | TKtexreg String
 | TKvertreg String
 | TKdwxyw
 
 | TKselector String
 | TKinstSat
 | TKinstX2
 | TKinstD8
 
 | TKcbracket
 | TKobracket
 
 | TKcomment String
 | TKcomma
 | TKfloat String
 | TKint Int
 | TKplus
 | TKminus
 
 | TKEOF deriving Show


comment :: AlexInput -> Int -> P Terminal
comment (inp, line) _ = do
  let (str, rest) = span (/= '\n') inp
  S.put (rest, line)
  return $ TKcomment str


tkConst :: Terminal -> AlexInput -> Int -> P Terminal
tkConst tk _ _ = return tk


tkValue :: (String -> Terminal) -> AlexInput -> Int -> P Terminal
tkValue op (inp, _) len =
  return $ op (take len inp)


type AlexInput = (String, Int) -- remaining input, line number

type P a = StateT AlexInput (ErrorT String Identity) a


alexGetChar :: AlexInput -> Maybe (Char, AlexInput)
alexGetChar ("", _) = Nothing  -- EOF
alexGetChar ((x:xs), oldLine) =
  Just (x, newInput)
  where
    newInput = (xs, newLine)
    newLine =
      case x of
        '\n' -> oldLine + 1  -- FIXME: make strict
        _    -> oldLine


alexInputPrevChar :: AlexInput -> Char
alexInputPrevChar _ = undefined


mkInput :: String -> AlexInput
mkInput str = (str, 1)


failP :: String -> P a
failP msg = do
  (_, line) <- S.get
  lift $ throwError $ "Error at line " ++ (show line) ++ ":\n" ++ msg


get :: P Terminal
get = do
  inp <- S.get
  case alexScan inp 0 of
    AlexEOF                -> return TKEOF
    AlexError (inp, _)     -> failP $ "lexical error: \"" ++ (takeWhile (/= '\n') inp) ++ "\""
    AlexSkip i len         -> do S.put i
                                 get
    AlexToken i len action -> do S.put i
                                 action inp len

    -- action :: AlexInput -> Int -> P Terminal

}
