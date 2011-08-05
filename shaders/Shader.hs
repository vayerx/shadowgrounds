module Shader where
import Data.List (intersperse)


data Reg
 = BaseReg String
 | RegSel String String
 | RegAddr String String Int
 | RegAddrSel String String Int String
 | OneMinus String
 | Minus String String
 deriving (Show)

data Stmt
 = Abs Reg Reg
 | Add Reg Reg Reg
 | AddSat Reg Reg Reg
 | Arl Reg Reg
 | Comment String
 | Cmp Reg Reg Reg Reg
 | Dcl Reg
 | Dcl2D Reg
 | DclColor Reg
 | DclNormal Reg
 | DclPosition Reg
 | DclTexcoord Int Reg
 | Def Reg String String String String
 | Dp3 Reg Reg Reg
 | Dp4 Reg Reg Reg
 | Lrp Reg Reg Reg Reg
 | M3x3 Reg Reg Reg
 | M4x3 Reg Reg Reg
 | M4x4 Reg Reg Reg
 | Mad Reg Reg Reg Reg
 | MadSat Reg Reg Reg Reg
 | Max Reg Reg Reg
 | Min Reg Reg Reg
 | Mov Reg Reg
 | MovD8 Reg Reg
 | Mul Reg Reg Reg
 | Mul2X Reg Reg Reg
 | Mul2XSat Reg Reg Reg
 | Rcp Reg Reg
 | Rsq Reg Reg
 | Sub Reg Reg Reg
 | SubSat Reg Reg Reg
 | Tex Reg
 | Texbem Reg Reg
 | Texcrd Reg Reg
 | Texkill Reg
 | Texld Reg Reg Reg
 | Texldp Reg Reg Reg
 | TexldDiv Reg Reg Reg
   deriving (Show)

data Shader
 = PixelShader [Stmt]
 | VertexShader [Stmt]


instance Show Shader where
  show (VertexShader stmts) = "Vertex shader:\n" ++ concat (intersperse "\n" (map show stmts))
  show (PixelShader stmts) = "Pixel Shader:\n" ++ concat (intersperse "\n" (map show stmts))

