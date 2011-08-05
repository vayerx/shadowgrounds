module Main(main) where
import Control.Exception (catch)
import Data.List (intersperse, isPrefixOf)
import Data.Map as Map (Map, empty, insert, keysSet, partition, singleton, toList, unionsWith)
import qualified Data.Map as Map (null)
import Data.Set as Set (Set, difference, empty, fromList, partition, toList, union)
import qualified Data.Set as Set (null)
import Prelude hiding (catch)
import System.Environment (getArgs)
import System.FilePath (takeFileName)


import Parser
import Shader


main :: IO ()
main = do
  args0 <- getArgs
  if null args0
    then putStrLn "usage: compile program [program] ..."
    else mapM_ compileShader args0


compileShader :: String -> IO ()
compileShader fileName = do
  putStrLn $ "Converting " ++ fileName ++ "..."
  contents <- readFile fileName
  case parseShader contents of
    Left err     -> putStrLn err
    Right shader -> do catch (writeFile ("glshaders/" ++ (takeFileName fileName)) (convertShader shader)) (\e -> putStrLn $ "error: " ++ (show e))

convertShader :: Shader -> String
convertShader (VertexShader stmts) =
  convertStatements "END" inputs stmts (
    "!!ARBvp1.0\n"
    ++ "#" ++ (show inputs) ++ "\n")
  where
    inputs = getRegisters stmts
convertShader (PixelShader stmts) =
  convertStatements "MOV outColor, r0;\nEND" inputs stmts (
    "!!ARBfp1.0\n"
    ++ (concatMap getInput (Set.toList inputs))
    ++ "OUTPUT outColor = result.color;\n")
  where
    inputs = getRegisters stmts
    getInput :: String -> String
    getInput inp =
      getInput' inputRegs
      where
        getInput' :: [(String, String -> String)] -> String
        getInput' [] = "# unknown input: " ++ inp ++ "\n"
        getInput' ((nm, func):xs) =
          if isPrefixOf nm inp
            then func (drop (length nm) inp)
            else getInput' xs


inputRegs :: [(String, String -> String)]
inputRegs = [("c", \x -> "PARAM c" ++ x ++ " = program.env[" ++ x ++ "];\n")
            ,("two", const "PARAM two = { 2.0, 2.0, 2.0, 2.0 };\n")
            ,("t", \x -> "ATTRIB t" ++ x ++ " = fragment.texcoord[" ++ x ++ "];\n")
            ,("v0", const "ATTRIB v0 = fragment.color.primary;\n")
            ,("v1", const "ATTRIB v1 = fragment.color.secondary;\n")
            ,("onePerEight", const "PARAM onePerEight = { 0.125, 0.125, 0.125, 0.125 };\n")]


-- used-before-definition
getRegisters :: [Stmt] -> Set String
getRegisters [] = Set.empty
getRegisters (x:xs) =
  Set.union (Set.difference afterUs (Set.fromList $ baseRegs defines)) (Set.fromList $ baseRegs uses)
  where
    baseRegs :: [Reg] -> [String]
    baseRegs = map baseReg
    baseReg :: Reg -> String
    baseReg (BaseReg r) = r
    baseReg (RegSel r _) = r
    baseReg (RegAddr r _ _) = r
    baseReg (RegAddrSel r _ _ _) = r
    baseReg (OneMinus r) = r
    baseReg (Minus r _) = r
    -- remove those defined by this
    -- and add those used by this
    (defines, uses) =
      case x of
        Abs dst src               -> ([dst], [src])
        Add dst src0 src1         -> ([dst], [src0, src1])
        AddSat dst src0 src1      -> ([dst], [src0, src1])
        Arl dst src               -> ([dst], [src])
        Comment _                 -> ([], [])
        Cmp dst src0 src1 src2    -> ([dst], [src0, src1, src2])
        Dcl dst                   -> ([dst], [])
        Dcl2D dst                 -> ([dst], [])
        DclColor dst              -> ([dst], [])
        DclNormal dst             -> ([dst], [])
        DclPosition dst           -> ([dst], [])
        DclTexcoord _ dst         -> ([dst], [])
        Def dst _ _ _ _           -> ([dst], [])
        Dp3 dst src0 src1         -> ([dst], [src0, src1])
        Dp4 dst src0 src1         -> ([dst], [src0, src1])
        Lrp dst src0 src1 src2    -> ([dst], [src0, src1, src2])
        M3x3 dst src0 src1        -> ([dst], [src0, src1])
        M4x3 dst src0 src1        -> ([dst], [src0, src1])
        M4x4 dst src0 src1        -> ([dst], [src0, src1])
        Mad dst src0 src1 src2    -> ([dst], [src0, src1, src2])
        MadSat dst src0 src1 src2 -> ([dst], [src0, src1, src2])
        Max dst src0 src1         -> ([dst], [src0, src1])
        Min dst src0 src1         -> ([dst], [src0, src1])
        Mov dst src               -> ([dst], [src])
        MovD8 dst src             -> ([dst], [src, BaseReg "onePerEight"])
        Mul dst src0 src1         -> ([dst], [src0, src1])
        Mul2X dst src0 src1       -> ([dst], [src0, src1, BaseReg "two"])
        Mul2XSat dst src0 src1    -> ([dst], [src0, src1, BaseReg "two"])
        Rcp dst src               -> ([dst], [src])
        Rsq dst src               -> ([dst], [src])
        Sub dst src0 src1         -> ([dst], [src0, src1])
        SubSat dst src0 src1      -> ([dst], [src0, src1])
        Tex dst                   -> ([dst], [])
        Texbem dst src            -> ([dst], [src])
        Texcrd dst src            -> ([dst], [src])
        Texkill src               -> ([], [src])
        Texld dst src0 _          -> ([dst], [src0])
        Texldp dst src0 src1      -> ([dst], [src0, src1])
        TexldDiv dst src0 _       -> ([dst], [src0])
        
    afterUs = getRegisters xs


outputRegs :: [(String, String, String)]
outputRegs = [("oPos", "result.position",        ""),
              ("oFog", "result.fogcoord",        ""),
              ("oPts", "result.pointsize",       ""),
              ("oC0",  "result.color",           ""),
              ("oD0",  "result.color.primary",   ""),
              ("oD1",  "result.color.secondary", ""),
              ("oT",   "result.texcoord[",       "]")]


genOutput :: String -> String
genOutput str =
  checkOut outputRegs
  where
    checkOut :: [(String, String, String)] -> String
    checkOut [] = error $ "genOutput: unknown register " ++ str
    checkOut ((x, gl, end):xs) =
      if isPrefixOf x str
        then "OUTPUT " ++ str ++ " = " ++ gl ++ (drop (length x) str) ++ end ++ ";\n"
        else checkOut xs


data RegType
 = Addr
 | Const Int
 | Temp deriving (Eq, Show)

convertStatements :: String -> Set String -> [Stmt] -> String -> String
convertStatements end inputs stmts acc =
  (convertStatements' (acc ++ declVars) stmts) ++ end
  where
    declVars =
      (if not $ Set.null tempVars
        then "TEMP " ++ (concat (intersperse ", " (Set.toList (Set.difference tempVars inputs)))) ++ ";\n"
        else "")
      ++ (if not $ Set.null addrVars
            then ("ADDRESS " ++ (concat (intersperse ", " (Set.toList addrVars))) ++ ";\n")
            else "")
      ++ (if not $ Map.null inVars
            then concatMap (\(name, (Const n)) -> ("PARAM " ++ name ++ "[" ++ (shows (n + 1)) ("] = { program.env[0.." ++ (shows n "] };\n")))) (Map.toList inVars)
            else "")
      ++ (concatMap genOutput (Set.toList outVars))
    (outVars, tempVars) = Set.partition (\x -> (head x) == 'o') (Map.keysSet nonInVars)
    (nonInVars, inVars) =
      Map.partition (== Temp) nonAddrVars
    (addrVars, nonAddrVars) =
      (\(a, b) -> (Map.keysSet a, b)) (Map.partition (== Addr) varSet)
    varSet :: Map String RegType
    varSet =
      Map.unionsWith combineConst (map grabRegs stmts)
      where
        combineConst (Const n1) (Const n2) = Const (max n1 n2)
        combineConst Addr _ = Addr
        combineConst _ Addr = Addr
        combineConst Temp (Const n) = Const n
        combineConst (Const n) Temp = Const n
        combineConst Temp Temp = Temp
        --combineConst t1 t2 = error $ "combineConst: mismatched types " ++ (show t1) ++ " and " ++ (show t2)
    
    grabRegs :: Stmt -> Map String RegType
    grabRegs (Abs r s) = dstTempSrcs r [s]
    grabRegs (Add r s1 s2) = dstTempSrcs r [s1, s2]
    grabRegs (AddSat r s1 s2) = dstTempSrcs r [s1, s2]
    grabRegs (Mov (RegSel "a0" _) _) = Map.singleton "a0" Addr
    grabRegs (Cmp r s1 s2 s3) = dstTempSrcs r [s1, s2, s3]
    grabRegs (Comment _) = Map.empty
    grabRegs (Dcl _) = Map.empty
    grabRegs (Dcl2D _) = Map.empty
    grabRegs (DclColor _) = Map.empty
    grabRegs (DclNormal _) = Map.empty
    grabRegs (DclPosition _) = Map.empty
    grabRegs (DclTexcoord _ _) = Map.empty
    grabRegs (Def _ _ _ _ _ ) = Map.empty
    grabRegs (Dp3 r s1 s2) = dstTempSrcs r [s1, s2]
    grabRegs (Dp4 r s1 s2) = dstTempSrcs r [s1, s2]
    grabRegs (Lrp r s1 s2 s3) = dstTempSrcs r [s1, s2, s3]
    grabRegs (M3x3 r s1 s2) = dstTempSrcs r [s1, (addReg 2 s2)]
    grabRegs (M4x3 r s1 s2) = dstTempSrcs r [s1, (addReg 2 s2)]
    grabRegs (M4x4 r s1 s2) = dstTempSrcs r [s1, (addReg 3 s2)]
    grabRegs (Mad r s1 s2 s3) = dstTempSrcs r [s1, s2, s3]
    grabRegs (MadSat r s1 s2 s3) = dstTempSrcs r [s1, s2, s3]
    grabRegs (Max r s1 s2) = dstTempSrcs r [s1, s2]
    grabRegs (Min r s1 s2) = dstTempSrcs r [s1, s2]
    grabRegs (Mov r s) = dstTempSrcs r [s]
    grabRegs (MovD8 r s) = dstTempSrcs r [s]
    grabRegs (Mul r s1 s2) = dstTempSrcs r [s1, s2]
    grabRegs (Mul2XSat r s1 s2) = dstTempSrcs r [s1, s2]
    grabRegs (Rcp r s) = dstTempSrcs r [s]
    grabRegs (Rsq r s) = dstTempSrcs r [s]
    grabRegs (Sub r s1 s2) = dstTempSrcs r [s1, s2]
    grabRegs (SubSat r s1 s2) = dstTempSrcs r [s1, s2]
    grabRegs (Tex r) = dstTemp r
    grabRegs (Texcrd r _) = dstTemp r
    grabRegs (Texbem r _) = dstTemp r
    grabRegs (Texkill _) = Map.empty
    grabRegs (Texld r _ _) = dstTemp r
    grabRegs (Texldp r _ _) = dstTemp r
    grabRegs (TexldDiv r _ _) = dstTemp r
    grabRegs stmt@_ = error $ "grabRegs: unknown stmt: " ++ (show stmt)


addReg :: Int -> Reg -> Reg
addReg 0 r = r
addReg n r = addReg (n - 1) (nextReg r)


dstTemp :: Reg -> Map String RegType
dstTemp r = Map.singleton (regName r) Temp


dstTempSrcs :: Reg -> [Reg] -> Map String RegType
dstTempSrcs r [] = dstTemp r
dstTempSrcs r (x:xs) =
  case x of
    RegAddr base _ n      -> Map.insert base (Const n) (dstTempSrcs r xs)
    RegAddrSel base _ n _ -> Map.insert base (Const n) (dstTempSrcs r xs)
    _                     -> dstTempSrcs r xs


regName :: Reg -> String
regName (BaseReg nm) = nm
regName (RegSel nm _) = nm
regName (RegAddr nm _ _) = nm
regName (RegAddrSel nm _ _ _) = nm
regName (OneMinus nm) = nm
regName (Minus nm _) = nm


convertStatements' :: String -> [Stmt] -> String
convertStatements' acc [] = acc
convertStatements' acc (instr:xs) =
  convertStatements' (acc ++ newStmt ++ "\n") xs
  where
    newStmt =
      case instr of
        Abs dst src        -> "ABS " ++ (regStr dst) ++ ", " ++ (regStr src) ++ ";"
        Add dst src0 src1  -> "ADD " ++ (regStr dst) ++ ", " ++ (srcRegStr src0) ++ ", " ++ (srcRegStr src1) ++ ";"
        AddSat dst src0 src1 -> "ADD_SAT " ++ (regStr dst) ++ ", " ++ (srcRegStr src0) ++ ", " ++ (srcRegStr src1) ++ ";"
        Arl dst src        -> "ARL " ++ (regStr dst) ++ ", " ++ (regStr src) ++ ";"
        Cmp dst src0 src1 src2 -> "CMP " ++ (regStr dst) ++ ", " ++ (srcRegStr src0) ++ ", " ++ (srcRegStr src1) ++ ", " ++ (srcRegStr src2) ++ ";"
        Comment str        -> '#':(drop 1 str)
        Dcl r              -> declare r
        DclColor r         -> "ATTRIB " ++ (regStr r) ++ " = vertex.color.primary;"
        DclNormal r        -> "ATTRIB " ++ (regStr r) ++ " = vertex.normal;"
        DclPosition r      -> "ATTRIB " ++ (regStr r) ++ " = vertex.position;"
        DclTexcoord n r    -> "ATTRIB " ++ (regStr r) ++ " = vertex.texcoord[" ++ (show n) ++ "];"
        Def dst x y z w    -> "PARAM " ++ (regStr dst) ++ " = {" ++ x ++ ", " ++ y ++ ", " ++ z ++ ", " ++ w ++ "};"
        Dp3 dst src0 src1  -> "DP3 " ++ (regStr dst) ++ ", " ++ (srcRegStr src0) ++ ", " ++ (srcRegStr src1) ++ ";"
        Dp4 dst src0 src1  -> "DP4 " ++ (regStr dst) ++ ", " ++ (srcRegStr src0) ++ ", " ++ (srcRegStr src1) ++ ";"
        Lrp dst src0 src1 src2 -> "LRP " ++ (regStr dst) ++ ", " ++ (srcRegStr src0) ++ ", " ++ (srcRegStr src1) ++ ", " ++ (srcRegStr src2) ++ ";"
        Mad dst src0 src1 src2 -> "MAD " ++ (regStr dst) ++ ", " ++ (srcRegStr src0) ++ ", " ++ (srcRegStr src1) ++ ", " ++ (srcRegStr src2) ++ ";"
        MadSat dst src0 src1 src2 -> "MAD_SAT " ++ (regStr dst) ++ ", " ++ (srcRegStr src0) ++ ", " ++ (srcRegStr src1) ++ ", " ++ (srcRegStr src2) ++ ";"
        Max dst src0 src1  -> "MAX " ++ (regStr dst) ++ ", " ++ (srcRegStr src0) ++ ", " ++ (srcRegStr src1) ++ ";"
        Min dst src0 src1  -> "MIN " ++ (regStr dst) ++ ", " ++ (srcRegStr src0) ++ ", " ++ (srcRegStr src1) ++ ";"
        M3x3 dst src0 src1 -> let (d, s0) = (regStr dst, regStr src0)
                              in    "DP3 " ++ d ++ ".x, " ++ s0 ++ ", " ++ (regStr src1) ++ ";\n"
                                 ++ "DP3 " ++ d ++ ".y, " ++ s0 ++ ", " ++ (regStr $ nextReg src1) ++ ";\n"
                                 ++ "DP3 " ++ d ++ ".z, " ++ s0 ++ ", " ++ (regStr $ nextReg $ nextReg src1) ++ ";"
        M4x3 dst src0 src1 -> let (d, s0) = (regStr dst, regStr src0)
                              in    "DP4 " ++ d ++ ".x, " ++ s0 ++ ", " ++ (regStr src1) ++ ";\n"
                                 ++ "DP4 " ++ d ++ ".y, " ++ s0 ++ ", " ++ (regStr $ nextReg src1) ++ ";\n"
                                 ++ "DP4 " ++ d ++ ".z, " ++ s0 ++ ", " ++ (regStr $ nextReg $ nextReg src1) ++ ";"
        M4x4 dst src0 src1 -> let (d, s0) = (regStr dst, regStr src0)
                              in    "DP4 " ++ d ++ ".x, " ++ s0 ++ ", " ++ (regStr src1) ++ ";\n"
                                 ++ "DP4 " ++ d ++ ".y, " ++ s0 ++ ", " ++ (regStr $ nextReg src1) ++ ";\n"
                                 ++ "DP4 " ++ d ++ ".z, " ++ s0 ++ ", " ++ (regStr $ nextReg $ nextReg src1) ++ ";\n"
                                 ++ "DP4 " ++ d ++ ".w, " ++ s0 ++ ", " ++ (regStr $ nextReg $ nextReg $ nextReg src1) ++ ";\n"
        Mov (RegSel "a0" _) src -> "ARL a0.x, " ++ (regStr src) ++ ";"
        Mov dst src        -> "MOV " ++ (regStr dst) ++ ", " ++ (srcRegStr src) ++ ";"
        MovD8 dst src      -> "MUL " ++ (regStr dst) ++ ", " ++ (srcRegStr src) ++ ", onePerEight;"
        Mul dst src0 src1  -> "MUL " ++ (regStr dst) ++ ", " ++ (srcRegStr src0) ++ ", " ++ (srcRegStr src1) ++ ";"
        Mul2XSat dst src0 src1 ->   "MUL " ++ (regStr dst) ++ ", " ++ (srcRegStr src0) ++ ", " ++ (srcRegStr src1) ++ ";\n"
                                 ++ "MUL_SAT " ++ (regStr dst) ++ ", " ++ (srcRegStr dst) ++ ", two;"
        Rcp dst src        -> "RCP " ++ (regStr dst) ++ ", " ++ (regStr src) ++ ";"
        Rsq dst src        -> "RSQ " ++ (regStr dst) ++ ", " ++ (regStr src) ++ ";"
        Sub dst src0 src1  -> "SUB " ++ (regStr dst) ++ ", " ++ (srcRegStr src0) ++ ", " ++ (srcRegStr src1) ++ ";"
        SubSat dst src0 src1 -> "SUB_SAT " ++ (regStr dst) ++ ", " ++ (srcRegStr src0) ++ ", " ++ (srcRegStr src1) ++ ";"
        Tex (BaseReg ('t':src)) -> "TEX t" ++ src ++ ", fragment.texcoord[" ++ src ++ "], texture[" ++ src ++ "], 2D;"
        Texcrd dst src     -> "MOV " ++ (regStr dst) ++ ", " ++ (srcRegStr src) ++ ";"
        Texkill dst        -> "KIL " ++ (regStr dst) ++ "; #FIXME: verify last component!"
        Texld dst src0 (BaseReg ('s':src1)) -> "TEX " ++ (regStr dst) ++ ", " ++ (regStr src0) ++ ", texture[" ++ src1 ++ "], 2D;"
        TexldDiv dst src0 (BaseReg ('s':src1)) -> "TXP " ++ (regStr dst) ++ ", " ++ (regStr src0) ++ ", texture[" ++ src1 ++ "], 2D;"
        Texldp dst src0 (BaseReg ('s':src1)) -> "TXP " ++ (regStr dst) ++ ", " ++ (regStr src0) ++ ", texture[" ++ src1 ++ "], 2D;"
        _                  -> "#FIXME: unknown statement: " ++ (show instr) ++ ";"


declare :: Reg -> String
declare (BaseReg (str@('t':num))) =
  "ATTRIB " ++ str ++ " = fragment.texcoord[" ++ num ++ "];"
declare (BaseReg "v0") =
  "ATTRIB v0 = fragment.color.primary;"
declare (RegSel (str@('t':num)) _) =
  "ATTRIB " ++ str ++ " = fragment.texcoord[" ++ num ++ "];"
declare reg = error $ "Bad declare: " ++ (show reg)


nextReg :: Reg -> Reg
nextReg (RegAddr base idx n) = RegAddr base idx (n + 1)
nextReg str = error $ "nextReg: unknown reg " ++ (show str)


srcRegStr :: Reg -> String
srcRegStr (RegSel base ".xy") = regStr (RegSel base ".xyyy")
srcRegStr (RegSel base ".xz") = regStr (RegSel base ".xzzz")
srcRegStr (RegSel base ".xyz") = regStr (RegSel base ".xyzz")
srcRegStr (RegSel base ".zw") = regStr (RegSel base ".zwww")
srcRegStr (RegSel base ".x") = regStr (RegSel base ".xxxx")
srcRegStr (RegSel base ".z") = regStr (RegSel base ".zzzz")
srcRegStr (RegSel base ".rgb") = regStr (RegSel base ".rgbb")
srcRegStr (RegAddrSel base idxReg idx ".xyz") = regStr (RegAddrSel base idxReg idx ".xyzz")
srcRegStr (RegAddrSel base idxReg idx ".xy") = regStr (RegAddrSel base idxReg idx ".xyyy")
srcRegStr (RegAddrSel base idxReg idx ".zw") = regStr (RegAddrSel base idxReg idx ".zwww")
srcRegStr r = regStr r


regStr :: Reg -> String
regStr (BaseReg base) = base
regStr (RegSel base sel) = base ++ sel
regStr (RegAddr base "" addr) = base ++ ('[':(shows addr "]"))
regStr (RegAddr base idxReg idx) = base ++ ('[':(idxReg ++ " + " ++ (shows idx "]")))
regStr (RegAddrSel base "" idx sel) = base ++ ('[':(shows idx  ("]" ++ sel)))
regStr (RegAddrSel base idxReg idx sel) = base ++ ('[':idxReg ++ " + " ++ (shows idx  ("]" ++ sel)))
regStr (OneMinus base) = "1-" ++ base
regStr (Minus base sel) = "-" ++ base ++ sel


