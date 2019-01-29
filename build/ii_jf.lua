local jf={}

function jf.trigger(...)ii.set(112,1,...)end
function jf.run_mode(...)ii.set(112,2,...)end
function jf.run(...)ii.set(112,3,...)end
function jf.transpose(...)ii.set(112,4,...)end
function jf.vtrigger(...)ii.set(112,5,...)end
function jf.retune(...)ii.set(112,6,...)end
function jf.mode(...)ii.set(112,7,...)end
function jf.play_voice(...)ii.set(112,8,...)end
function jf.play_note(...)ii.set(112,9,...)end
function jf.god_mode(...)ii.set(112,10,...)end
function jf.tick(...)ii.set(112,11,...)end
function jf.quantize(...)ii.set(112,12,...)end
jf.g={
	['run_mode']=130,
	['run']=131,
	['transpose']=132,
	['mode']=135,
	['god_mode']=138,
	['tick']=139,
	['quantize']=140,
	['retune']=134,
}
function jf.get(name,...)ii.get(112,jf.g[name],...)end
jf.e={
	[130]='run_mode',
	[131]='run',
	[132]='transpose',
	[135]='mode',
	[138]='god_mode',
	[139]='tick',
	[140]='quantize',
	[134]='retune',
}
return jf
