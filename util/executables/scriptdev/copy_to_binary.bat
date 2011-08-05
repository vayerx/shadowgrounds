if exist ..\..\..\..\aov\binary\tools goto aov

copy Release\scriptdev.exe ..\..\..\..\Action_RPG\binary\tools\*.*
copy Release\scriptdev.exe ..\..\..\..\asg_proto\binary\tools\*.*
rem copy Release\scriptdev.exe ..\..\..\..\Shadowgrounds\binary\tools\*.*
copy Release\scriptdev.exe ..\..\..\..\sg_binary\*.*
copy Release\scriptdev.exe ..\..\..\..\sg_physics\*.*

goto done

:aov

copy Release\scriptdev.exe ..\..\..\..\aov\binary\tools\*.*

:done
