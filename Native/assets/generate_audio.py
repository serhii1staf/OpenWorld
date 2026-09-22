"""Reproducible original synthetic sound assets. Python standard library only."""
import math, random, struct, wave
from pathlib import Path
ROOT=Path(__file__).parent
RATE=22050
random.seed(78412)

def write(name, seconds, func, loop=False):
    total=int(RATE*seconds)
    data=bytearray()
    state=0.0
    for i in range(total):
        t=i/RATE
        noise=random.uniform(-1,1)
        state=state*.97+noise*.03
        value=func(t,noise,state)
        if loop:
            value*=min(1.,t/.04,(seconds-t)/.04)
        value=max(-.95,min(.95,value))
        data+=struct.pack('<h',int(value*32767))
    with wave.open(str(ROOT/(name+'.wav')),'wb') as f:
        f.setnchannels(1);f.setsampwidth(2);f.setframerate(RATE);f.writeframes(data)
    if loop:
        (ROOT/(name+'.wav.import')).write_text('''[remap]\nimporter="wav"\ntype="AudioStreamWAV"\n\n[deps]\nsource_file="res://assets/'''+name+'''.wav"\n\n[params]\nloop/mode=2\nloop/begin=0\nloop/end=-1\n''')

write('shot',.24,lambda t,n,s:(n*.65+math.sin(t*math.tau*90)*.35)*math.exp(-t*24))
write('step',.15,lambda t,n,s:n*.28*math.exp(-t*30)+math.sin(t*math.tau*120)*.22*math.exp(-t*40))
write('reload',.5,lambda t,n,s:n*.25*(math.exp(-abs(t-.06)*100)+math.exp(-abs(t-.31)*90)))
write('impact',.45,lambda t,n,s:(n*.4+math.sin(t*math.tau*50)*.6)*math.exp(-t*12))
write('success',1.2,lambda t,n,s:.2*sum(math.sin(t*math.tau*f) for f in [440,554.365,659.255])/3*math.exp(-t*2)*min(t*80,1))
write('engine',2,lambda t,n,s:.17*math.sin(t*math.tau*50)+.07*math.sin(t*math.tau*100)+s*.12,True)
write('ambient',8,lambda t,n,s:s*.9*(.5+.3*math.sin(t*math.tau/8))+n*.016,True)
chords=[[146.832,220,293.664,369.994],[130.813,196,261.626,329.628],[164.814,246.942,329.628,415.305],[110,164.814,220,277.183]]
def music(t,n,s):
    index=int(t//4)%4; local=t%4
    envelope=min(1,local/1.0,(4-local)/1.1)
    return sum(math.sin(t*math.tau*f)*.12+math.sin(t*math.tau*f*2)*.02 for f in chords[index])*envelope*.4
write('music',16,music,True)
print('Generated original audio assets.')
