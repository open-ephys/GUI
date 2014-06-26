function [afTargetTS] = fnTimeZoneTransform(strctSync,afSourceTS)

afTargetTS= ((afSourceTS - strctSync.sourceT0) * strctSync.coeff(2) + strctSync.coeff(1)) + strctSync.targetT0;
