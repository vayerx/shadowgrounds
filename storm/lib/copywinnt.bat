
rem --- OLD SG STUFF ---
rem copy ..\lib\storm3dv2.dll ..\..\..\sg_binary



rem --- AOV ---
if exist ..\..\..\aov\binary goto aov
goto aov_done
:aov
  copy ..\lib\storm3dv2.dll ..\..\..\aov\binary
:aov_done

rem --- SG PHYSICS TRUNK ---
if exist ..\..\..\sg_physics\trunk goto trunk
goto trunk_done
:trunk
  copy ..\lib\storm3dv2.dll ..\..\..\sg_physics\trunk
:trunk_done

rem --- SG PHYSICS (NO TRUNK) ---
if exist ..\..\..\sg_physics goto sg_physics
goto sg_physics_done
:sg_physics
  copy ..\lib\storm3dv2.dll ..\..\..\sg_physics
:sg_physics_done

rem --- SURVIVOR ---
if exist ..\..\..\survivor\binary goto survivor
goto survivor_done
:survivor
  copy ..\lib\storm3dv2.dll ..\..\..\survivor\binary
:survivor_done

rem --- SURVIVOR ---
if exist ..\..\..\survivor\snapshot goto survivor2
goto survivor_done2
:survivor2
  copy ..\lib\storm3dv2.dll ..\..\..\survivor\snapshot
:survivor_done2

rem --- CLAW PROTO ---
if exist ..\..\..\claw_proto\binary goto claw_proto
goto claw_proto_done
:claw_proto
  copy ..\lib\storm3dv2.dll ..\..\..\claw_proto\binary
:claw_proto_done

