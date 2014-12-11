// ----------------------------------------------------------------------------
// Copyright (C) 2014 Clint Weisbrod. All rights reserved.
//
// constants.h
//
// THIS SOFTWARE IS PROVIDED BY CLINT WEISBROD "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL CLINT WEISBROD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ----------------------------------------------------------------------------

#pragma once


// Mathematical constants
#define PI 3.14159265358979323846
const double	kPi = PI;
const double	kOneOverPi = 1.0 / PI;
const double	kTwicePi = 2.0*PI;
const double	kOneOverTwicePi = 1.0 / (2.0*PI);
const double	kOneOverFourPi = 1.0 / (4.0*PI);
const double	kHalfPi = PI / 2.0;
const double	kPiBy4 = PI / 4.0;
const double	k3PiBy2 = 3.0*PI / 2.0;
const double	kRadPerDegree = PI / 180.0;
const double	kDegPerRadian = 180.0 / PI;
const double	kRadiansPerArcMinute	= kRadPerDegree/60.0;
const double	kArcMinutesPerRadian	= 60.0*kDegPerRadian;
const double	kRadiansPerArcSecond	= 4.8481368110953599359e-6;		// number of radians in arc second
const double	kArcSecondsPerRadian	= 206264.8062470963552;			// number of arc seconds in a radian
const double 	kMASPerArcSecond 		= 1000.0;						// milli arc seconds per second is 1000
const double 	kArcSecondPerMAS 		= 1.0/kMASPerArcSecond;			// milli arc seconds per second is 1000
const double	kArcMinutesPerDeg		= 60.0;
const double	kArcSecondsPerDeg		= kArcMinutesPerDeg*60.0;		

// Time constants
const double	k2000JulianDate			=2451545.0;

const double	kYearsPerCentury=		100.0;
const int		kIntDaysPerCentury=		36525;
const double	kDaysPerCentury=		36525.0;
const double	kDaysPerYear=			365;				//.. figure out way to add leap years!
const double	kDaysPerSiderealDay=	86164.09/86400.0;	// from astro companion
const double	kDaysPerSiderealMonth=	27.3216605324;		// sidereal month on earth (Astronomy Made Simple p 31)
const double	kDaysPerSynodicMonth=	29.530588853;		// synodic month on earth (Meeus p 319)
const double	kSecondsPerCentury=		3155760000.0;
const double	kSecondsInMinute=		60.0;
const double	kMinutesPerHour=		60.0;
const double	kSecondsInHour=			3600.0;
const double	kSecondsInDay=			86400.0;
const double	kDaysInSecond=			1.0/86400.0;
const double	kCenturiesPerMinute=	kSecondsInMinute / kSecondsPerCentury;
const int		kTicksInDay=			5184000;
const double	kSecondsInHalfDay=		43200.0;
const int		kMonthsInYear=			12;
const double	kHoursInDay=			24.0;
const double	kDaysInJulianDay=		1.0;
const double	kDaysInHour=			1.0/24.0;
const double	kMinutesInDay=			1440.0;
const double	kDaysInMinute=			1.0/1440.0;
const double	kDaysPerWeek=			7.0;
const double	kDegreesInHour=			15.0;
const double	kMicrosecondsPerSecond=	1e6;		// million microsec per second, 60 ticks per second
const double	kMicrosecondsPerTick=	1e6/60.0;	// million microsec per second, 60 ticks per second
const double	kMicrosecondsPerMinute=	1e6*60.0;	// million microsec per second, 60 sec per min

const short		kJanuary=				1;
const short		kFebuary=				2;
const short		kMarch=					3;
const short		kApril=					4;
const short		kMay=					5;
const short		kJune=					6;
const short		kJuly=					7;
const short		kAugust=				8;
const short		kSeptember=				9;
const short		kOctober=				10;
const short		kNovember=				11;
const short		kDecember=				12;

// Physical constants
const double	kMetrePerKilometre		=1000.0;
const double	kLightYearsPerParsec	=3.261633;
const double	kParsecsPerLightYear	=1.0/kLightYearsPerParsec;
const double	kMetrePerAu				=1.4959787066e11;			// number of metres per astronomical unit
const double	kAuPerMetre				=1.0/kMetrePerAu;			// number of astronomical unit per metres
const double	kKilometersPerAu		=kMetrePerAu/kMetrePerKilometre;
const double	kAuPerKilometers		=6.68458712405579e-9;		//.. this # does not jive with astro-almanac sup. p.716
const double	kSpeedOfLight			=1.581250732e-7;			// measured as centuries per AU
const double	kSpeedOfLightPerMetre	=kAuPerMetre*kSpeedOfLight;	// measured as centuries per Metre
const double	kSpeedOfLightAUPerHour	=kDaysInHour/kDaysPerCentury/kSpeedOfLight;
const double 	kParsecsPerAu 			= 4.848136811e-6;			// explanatory suppl, page 716
const double 	kAuPerParsec 			= 1.0/kParsecsPerAu;		// explanatory suppl, page 716
const double 	kAuPerLightYear 		= kAuPerParsec/kLightYearsPerParsec;	
const double 	kLightYearPerAu 		= 1.0/kAuPerLightYear;
const double	kMetrePerLightYear		=kAuPerLightYear*kMetrePerAu;
const double	kLightYearPerMetre		= 1.0/kMetrePerLightYear;
const double	kCentimetrePerInch		=2.54;
const double	kMillimetrePerInch		=kCentimetrePerInch*10.0;
const double	kInchesPerMillimetre	=1.0/kMillimetrePerInch;
const double	kFeetPerYard			=3.0;
const double	kMetrePerFoot			=kCentimetrePerInch*12.0/100.0;
const double	kMetrePerMile			=kMetrePerFoot*5280.0;
const double	kMetrePerParsec			=kMetrePerLightYear*kLightYearsPerParsec;
const double 	kOneOverRootTwo 		=7.07106781187E-01;
const double	kAbsoluteZeroInCelcius	= -273.1;	// Alsolute zero,
const double	kGravitationalConstant	= 6.693e-11;
const double	kAltOfHorizonAtSeaLevel    = -0.5667*kRadPerDegree;
const double	kSinAltOfHorizonAtSeaLevel = -0.009890619607;
const double	kCosAltOfHorizonAtSeaLevel = 9.99951086626E-01;
const double	kSizeOfSolarSytemMeters = 760.0*kMetrePerAu;
const double	kDistanceToBigBang = 13500e6*kMetrePerAu*kAuPerLightYear;		// 13.5 Billion ly is the distance to the BB in Tully's cosmology.

// Astronomical constants
const double	kJ2000ObliquityAngle	=0.4090926292045900569;		// from Explanatory Sup 2 Astro Almanac p 696
const double	kJ2000CosObliquityAngle =0.9174821316872261842;		// from Explanatory Sup 2 Astro Almanac p 696
const double	kJ2000SinObliquityAngle =0.397776995356271641;		// from Explanatory Sup 2 Astro Almanac p 696

// Solar System Physical constants
const double	kRadiusOfEarthMetres		= 6378140;		// this seems to be close to our value
const double	kRadiusOfEarthKM			= kRadiusOfEarthMetres/kMetrePerKilometre;
const double	kRadiusOfEarthAU			= kRadiusOfEarthKM*kAuPerKilometers;
const double	kEarthRadiusAu				= kRadiusOfEarthAU;

const double	kRadiusOfMoonMetres			= 1738000;
const double	kRadiusOfMoonKM				= kRadiusOfMoonMetres/kMetrePerKilometre;
const double	kRadiusOfMoonAU				= kRadiusOfMoonKM*kAuPerKilometers;
const double	kMoonRadiusAu				= kRadiusOfMoonAU;

const double	kSunRadius					= 4.6524e-3;			// in au
const double	kMercuryRadius				= 2439.7*kAuPerKilometers;
const double	kVenusRadius				= 6051.9*kAuPerKilometers;
const double	kEarthRadius				= kRadiusOfEarthMetres*kAuPerMetre;
const double	kMoonRadius					= kRadiusOfMoonMetres*kAuPerMetre;
const double	kMarsRadius					= 3397*kAuPerKilometers;
const double	kPhobosRadius				= 10.8*kAuPerKilometers;
const double	kDeimosRadius				= 6.1*kAuPerKilometers;
const double	kJupiterRadius				= 71492*kAuPerKilometers;
const double	kIoRadius					= 1815*kAuPerKilometers;
const double	kEuropaRadius				= 1569*kAuPerKilometers;
const double	kGanymedeRadius				= 2631*kAuPerKilometers;
const double	kCallistoRadius				= 2400*kAuPerKilometers;
const double	kAmaltheaRadius				= 83*kAuPerKilometers;
const double	kHimaliaRadius				= 93*kAuPerKilometers;
const double	kElaraRadius				= 38*kAuPerKilometers;
const double	kSaturnRadius				= 60268*kAuPerKilometers;
const double	kMimasRadius				= 196*kAuPerKilometers;
const double	kEnceladusRadius			= 250*kAuPerKilometers;
const double	kTethysRadius				= 530*kAuPerKilometers;
const double	kDioneRadius				= 560*kAuPerKilometers;
const double	kRheaRadius					= 765*kAuPerKilometers;
const double	kTitanRadius				= 2575*kAuPerKilometers;
const double	kHyperionRadius				= 130*kAuPerKilometers;
const double	kIapetusRadius				= 730*kAuPerKilometers;
const double	kPhoebeRadius				= 110*kAuPerKilometers;
const double	kJanusRadius				= 100*kAuPerKilometers;
const double	kUranusRadius				= 25559*kAuPerKilometers;
const double	kArielRadius				= 579*kAuPerKilometers;
const double	kUmbrielRadius				= 586*kAuPerKilometers;
const double	kTitaniaRadius				= 790*kAuPerKilometers;
const double	kOberonRadius				= 762*kAuPerKilometers;
const double	kMirandaRadius				= 240*kAuPerKilometers;
const double	kNeptuneRadius				= 24764*kAuPerKilometers;
const double	kTritonRadius				= 1353*kAuPerKilometers;
const double	kNereidRadius				= 170*kAuPerKilometers;
const double	kPlutoRadius				= 1151*kAuPerKilometers;
const double	kCharonRadius				= 593*kAuPerKilometers;
const double	kErisRadius					= 1225*kAuPerKilometers; // From NinePlanets.org (Bill A).
const double	kCeresRadius				= (952.2/2)*kAuPerKilometers; // JPL
const double	kDysnomiaRadius				= 75*kAuPerKilometers;
const double	kHydraRadius				= 57*kAuPerKilometers; // very rough at 2008/02/12
const double	kNixRadius					= 45*kAuPerKilometers; // very rough at 2008/02/12

const double	kShuttleOrbitRadius			= 0.05*kAuPerKilometers;
const double	kComSatOneRadius			= 0.04*kAuPerKilometers;
const double	kMoonProbeRadius			= 0.03*kAuPerKilometers;

// Mass is from Green book, explanatory suppl. to the Astro Almanac, pg 697, 710
const double	kSunMassKgs					= 1.988435e30;	// in kilograms
const double	kSunMass					= 1.0;						// in solar masses
const double	kMercuryMass				= 1.0/6023600.0;
const double	kVenusMass					= 1.0/408523.5;
const double	kEarthMass					= 1.0/332996.3619114379;
const double	kMoonMass					= 0.0123002*kEarthMass;
const double	kMarsMass					= 1.0/3098710.0;				// I checked with the NASA MGS site, and they have it as 3098708.0, which is close enough for us to stay with what we got
const double	kPhobosMass					= 1.5e-8*kMarsMass;
const double	kDeimosMass					= 3e-9*kMarsMass;
const double	kJupiterMass				= 1.0/1047.355;
const double	kIoMass						= 4.68e-5*kJupiterMass;
const double	kEuropaMass					= 2.52e-5*kJupiterMass;
const double	kGanymedeMass				= 7.80e-5*kJupiterMass;
const double	kCallistoMass				= 5.66e-5*kJupiterMass;
const double	kAmaltheaMass				= 38e-10*kJupiterMass;
const double	kHimaliaMass				= 50e-10*kJupiterMass;
const double	kElaraMass					= 4e-10*kJupiterMass;
const double	kSaturnMass					= 1.0/3489.5;
const double	kMimasMass					= 8e-8*kSaturnMass;
const double	kEnceladusMass				= 1.3e-7*kSaturnMass;
const double	kTethysMass					= 1.3e-6*kSaturnMass;
const double	kDioneMass					= 1.85e-6*kSaturnMass;
const double	kRheaMass					= 4.4e-6*kSaturnMass;
const double	kTitanMass					= 2.38e-4*kSaturnMass;
const double	kHyperionMass				= 3e-8*kSaturnMass;
const double	kIapetusMass				= 3.3e-6*kSaturnMass;
const double	kPhoebeMass					= 7e-10*kSaturnMass;
const double	kJanusMass					= 1e-10*kSaturnMass;	// none given (small)
const double	kUranusMass					= 1.0/22869;
const double	kArielMass					= 1.56e-5*kUranusMass;	
const double	kUmbrielMass				= 1.35e-5*kUranusMass;	
const double	kTitaniaMass				= 4.06e-5*kUranusMass;	
const double	kOberonMass					= 3.47e-5*kUranusMass;	
const double	kMirandaMass				= 0.08e-5*kUranusMass;	
const double	kNeptuneMass				= 1.0/19314;
const double	kTritonMass					= 2.09e-4*kNeptuneMass;
const double	kNereidMass					= 2e-7*kNeptuneMass;
const double	kPlutoMass					= 1.0/3e6;
const double	kCharonMass					= 0.22*kPlutoMass;
const double	kNixMass					= 5.0e17 / kSunMassKgs;
const double	kHydraMass					= 5.0e17 / kSunMassKgs;
const double	kCeresMass					= 9.46e20 / kSunMassKgs;	// Wiki
const double	kErisMass					= 1.6e22 / kSunMassKgs;	// Wiki
const double	kDysnomiaMass				= 2.0e18 / kSunMassKgs;	// very rough estimate, assuming similar density as other dwarf planets and radius
const double	kHaumeaMass					= 4.2e21 / kSunMassKgs;	// Wiki
const double	kMakemakeMass				= 4e21 / kSunMassKgs;	// Wiki

const double 	kSolarVolumeConst			= 8*kSunRadius*kSunRadius*kSunRadius; // used to find mass of arbitrarily sized object
const double	kShuttleOrbitMass			= 8*kShuttleOrbitRadius*kShuttleOrbitRadius*kShuttleOrbitRadius/kSolarVolumeConst;
const double	kComSatOneMass				= 8*kComSatOneRadius*kComSatOneRadius*kComSatOneRadius/kSolarVolumeConst;
const double	kMoonProbeMass				= 8*kMoonProbeRadius*kMoonProbeRadius*kMoonProbeRadius/kSolarVolumeConst;

const double	kMassOfSunInKilograms		= 1.989e30;		// Dave got this from Bill A's "Nine Planets"
const double	kMassOfMercuryInKilograms	= 3.30e23;		
const double	kMassOfVenusInKilograms		= 4.87e24;		
const double	kMassOfEarthInKilograms		= 5.9742e24;
const double	kMassOfMoonInKilograms		= 7.3477e22;
const double	kMassOfMarsInKilograms		= 6.42e23;		
const double	kMassOfJupiterInKilograms	= 1.90e27;	
const double	kMassOfSaturnInKilograms	= 5.69e26;
const double	kMassOfUranusInKilograms	= 8.69e25;
const double	kMassOfNeptuneInKilograms	= 1.02e26;
const double	kMassOfPlutoInKilograms		= 1.31e22;

// Suggested sizes for the ring drawings. 
//-----------------------------------------
//
//A Ring (outer)
//Width = 0.244, radius = 2.143, length = 13.46, l/w = 55.18
//32 x 1766 pixmap
//
//B Ring (middle) 
//Width = 0.423, radius = 1.7345, length = 10.90, l/w = 25.76
//64 x 1649 pixmap
//
//CRing (inner)
//width = 0.29, radius = 1.378, length = 8.66, l/w = 29.86
//48 x 1433.8
const double	kFRingRadius				= 2.327*kSaturnRadius;
const double	kARingOuterRadius			= 2.265*kSaturnRadius;
const double	kARingInnerRadius			= 2.021*kSaturnRadius;
const double	kBRingOuterRadius			= 1.946*kSaturnRadius;
const double	kBRingInnerRadius			= 1.523*kSaturnRadius;
const double	kCRingOuterRadius			= 1.523*kSaturnRadius;
const double	kCRingInnerRadius			= 1.233*kSaturnRadius;

const double	kOuterMostRingRadius		= kARingOuterRadius;		// Radius of outermost ring - the A ring for now
const double	kInnerMostRingRadius		= kCRingInnerRadius;		// inner Radius of innermost ring - the C ring for now
