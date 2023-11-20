// JavaScript Document
function cl1(){
	var b=document.getElementById("b");
	var i=Math.floor(Math.random()*10);
	b.style.background="url(img/"+i+".jpg)";
	
	b.style.backgroundAttachment="fixed";      
}		// url("图片目录路径/前缀名'+ randomBgIndex +'.jpg");
function cl2(){
	var b=document.getElementById("b");
	var i=Math.floor(Math.random()*10);
	b.style.background="url(../img/"+i+".jpg)";
	//b.style.background="url(img/1.jpg)";
	b.style.backgroundAttachment="fixed";
     
}function cl3(){
	 var i=Math.floor(Math.random()*10);
	 var audioDOM = document.createElement('audio');
	 audioDOM.currentTime = 0;
	 audioDOM.src="music/"+i+".mp3";
	 audioDOM.play();
    document.getElementById("img").style.display='none';
}function cl4(){
	 var i=Math.floor(Math.random()*10);
	 var audioDOM = document.createElement('audio');
	 audioDOM.currentTime = 0;
	 audioDOM.src="../music/"+i+".mp3";
	 audioDOM.play();
    document.getElementById("img").style.display='none';
}