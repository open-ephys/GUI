function [strctSync, JitterStd,Jitter] = fnTimeZoneFit(afSourceTS, afTargetTS)
strctSync.sourceT0 = nanmean(afSourceTS);
strctSync.targetT0 = nanmean(afTargetTS);
strctSync.coeff = robustfit((afSourceTS-strctSync.sourceT0), (afTargetTS-strctSync.targetT0));
 
Jitter = afTargetTS- (((afSourceTS - strctSync.sourceT0) * strctSync.coeff(2) + strctSync.coeff(1)) + strctSync.targetT0);
JitterStd = nanstd(Jitter);
