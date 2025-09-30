import numpy as np
from math import sqrt,log
from scipy import special
from matplotlib import pyplot as plt

def voigt(x,amp,pos,fwhm,shape):
     """\
voigt profile

V(x,sig,gam) = Re(w(z))/(sig*sqrt(2*pi))
z = (x+i*gam)/(sig*sqrt(2))
     """

     tmp = 1/special.wofz(np.zeros((len(x))) \
           +1j*np.sqrt(np.log(2.0))*shape).real
     tmp = tmp*amp* \
           special.wofz(2*np.sqrt(np.log(2.0))*(x-pos)/fwhm+1j* \
           np.sqrt(np.log(2.0))*shape).real
     return tmp

xs = np.arange(-5.,5.,0.01)
width = 3.
a_gauss = width/(2*sqrt(log(2)))

# shapes:
gauss = np.exp(-(xs/a_gauss)**2)/(a_gauss*sqrt(np.pi))
asym = np.zeros(xs.size)
asym[xs<=0] = np.exp(-(xs[xs<=0]/((1-0.4)*a_gauss))**2)/(a_gauss*sqrt(np.pi))
asym[xs>=0] = np.exp(-(xs[xs>=0]/((1+0.4)*a_gauss))**2)/(a_gauss*sqrt(np.pi))
asym_low = np.zeros(xs.size)
asym_low[xs<=0] = np.exp(-(xs[xs<=0]/((1-0.2)*a_gauss))**2)/(a_gauss*sqrt(np.pi))
asym_low[xs>=0] = np.exp(-(xs[xs>=0]/((1+0.2)*a_gauss))**2)/(a_gauss*sqrt(np.pi))
lorentz1 = a_gauss**2/(xs**2+a_gauss**2)
lorentz2 = a_gauss**4/(xs**4+a_gauss**4)
voigtprof = voigt(xs,1.,0.,a_gauss,1.)

def makeplot(ydata,filename) :
    fig = plt.figure(figsize=(1.5,1.5/1.618)) # 2 inches wide, use golden ratio
    ax = fig.add_subplot(111)
    ax.spines['left'].set_color('none')
    ax.spines['right'].set_color('none')
    ax.spines['top'].set_color('none')
    ax.set_yticks([])
    ax.set_xticks([0])
    ax.set_xticklabels([''])
    ax.xaxis.set_ticks_position('bottom')
    plt.plot(xs,ydata,color='black')
    ax.set_ylim(0.,ydata.max()*1.1)
    fig.savefig(filename)

map(lambda (curve, filename) : makeplot(curve,filename),
    [(gauss,'./gauss.pdf'),
     (asym, './asym04.pdf'),
     (asym_low, './asym02.pdf'),
     (lorentz1, './lorentz1.pdf'),
     (lorentz2, './lorentz2.pdf'),
     (voigtprof, './voigt.pdf')])

