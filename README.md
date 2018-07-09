# Ray-Tracing

This project is built in C/C++ with IDE CodeBlocks when i took Advanced-CG in NTNU in 2018.  
The basic algorithm is backward ray-tracing Phong reflection model.  
The project is accelerated by KD-Tree which speeds up at least 100x when the number of recursion is 3.  
What is recursion light? You can see the object on another object's surface by the reflection.  
<img src="Picture/ball.jpg" width="200" >  
You can see the reflection on the surface of each ball.  

This project also surports rendering an 3D object which consists of numerous primitives.  
<img src="Picture/monkey512.jpg" width="200" >  
Primitive:900+, Resolution:512x512, Recursion:3, Runtime:12s  
<img src="Picture/bunny512.jpg" width="200" >  
Primitive:26000+, Resolution:512x512, Recursion:3, Runtime:50s  
The above is on the branch of Home  


# Load .obj and .mtl file  
The following is on the branch of HomeHome  
With the benefit of loading these two file, texture mapping can be done.  
Be carefull! In this project, there are still bugs in the texture mapping.  
<img src="Picture/Ambition.jpg" width="200" >  
Mapping an image to a plane is flawless.   
Wolmyeongdong,Daejeong City,Korea 

<img src="Picture/banana.jpg" width="200" >  
Banana  
<img src="Picture/sofa.jpg" width="200" >  
Sofa  
