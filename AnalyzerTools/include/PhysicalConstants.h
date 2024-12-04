#ifndef PhysicalConstants_h
#define PhysicalConstants_h
// S. Navas et al. (Particle Data Group), Phys. Rev. D 110, 030001 (2024)

//Masses (in GeV)
static constexpr float T_MASS = 172.57;
static constexpr float W_MASS = 80.3692;
static constexpr float Z_MASS = 91.1880;
static constexpr float H_MASS = 125.20;

//Decay widths (in GeV)
static constexpr float T_WIDTH = 1.42;
static constexpr float W_WIDTH = 2.085;
static constexpr float Z_WIDTH = 2.4955;
static constexpr float H_WIDTH = 3.7e-3;

//coupling constants
static constexpr float G_F = 1.1663788e-5; //GeV^-2
static constexpr float Alpha_S_MZ = 0.1180;

//CKM matrix
static constexpr float Vcb = 41.1e-3;
static constexpr float Vcs = 0.975;
static constexpr float Vcd = 0.221;
static constexpr float Vub = 3.82e-3;
static constexpr float Vud = 0.97367;
static constexpr float Vus = 0.22431;

#endif