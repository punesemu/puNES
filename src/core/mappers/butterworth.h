class butterworth {
private:
	double *A, *d1, *d2, *w0, *w1, *w2;
	int n;
	void flush();
public:
	double output(double);
	void recalc(int, double, double);
	butterworth(int, double, double);
	~butterworth();
};

class LPF_RC {
protected:    
	double	a0;
	double	b1;
	double	z1;
public:
	LPF_RC();
	LPF_RC(double);
	void	setFc (double);
	float	process (float);  
};

class HPF_RC: public LPF_RC {
protected:    
public:
	HPF_RC();
	HPF_RC(double);
	float	process (float);  
};
