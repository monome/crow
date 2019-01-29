local wslash={}

function wslash.record(...)ii.set(113,1,...)end
function wslash.play(...)ii.set(113,2,...)end
function wslash.loop(...)ii.set(113,3,...)end
function wslash.cue(...)ii.set(113,4,...)end
wslash.g={
	['record']=129,
	['play']=130,
	['loop']=131,
	['cue']=132,
}
function wslash.get(name,...)ii.get(113,wslash.g[name],...)end
wslash.e={
	[129]='record',
	[130]='play',
	[131]='loop',
	[132]='cue',
}
return wslash
