function [afSourceTS] = fnTimeZoneInvTransform(strctSync,afTargetTS)
afSourceTS=(afTargetTS-strctSync.targetT0-strctSync.coeff(1))/strctSync.coeff(2)+strctSync.sourceT0;
