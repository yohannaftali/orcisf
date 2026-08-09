#include "engine/StiffnessSolver.h"

namespace orcisf::engine {

void BandedFactorize(int N, int NB, LegacyArray2D<float>& A_SFF) {
    if (!(A_SFF[1][1] > 0)) {
        throw UnstableStructureError();
    }

    for (int jbf = 2; jbf <= N; ++jbf) {
        int J2 = jbf - NB + 1;
        if (J2 < 1) {
            J2 = 1;
        }
        if ((jbf - 1) != 1) {
            for (int ibf = 2; ibf <= jbf - 1; ++ibf) {
                if ((ibf - 1) >= J2) {
                    float SUM = A_SFF[ibf][jbf - ibf + 1];
                    for (int kbf = J2; kbf <= ibf - 1; ++kbf) {
                        SUM = SUM - A_SFF[kbf][ibf - kbf + 1] * A_SFF[kbf][jbf - kbf + 1];
                    }
                    A_SFF[ibf][jbf - ibf + 1] = SUM;
                } else {
                    continue;
                }
            }
        }
        float SUM = A_SFF[jbf][1];
        for (int kbf = J2; kbf <= jbf - 1; ++kbf) {
            float TEMP = A_SFF[kbf][jbf - kbf + 1] / A_SFF[kbf][1];
            SUM = SUM - TEMP * A_SFF[kbf][jbf - kbf + 1];
            A_SFF[kbf][jbf - kbf + 1] = TEMP;
        }
        if (SUM <= 0) {
            throw UnstableStructureError();
        }
        A_SFF[jbf][1] = SUM;
    }
}

void BandedSolve(int N, int NB, const LegacyArray2D<float>& U_SFF,
                  const LegacyArray<float>& B_AC, LegacyArray<float>& X_DF) {
    for (int ins = 1; ins <= N; ++ins) {
        X_DF[ins] = 0.f;
    }

    for (int ibsl = 1; ibsl <= N; ++ibsl) {
        int jbns = ibsl - NB + 1;
        if (ibsl <= NB) {
            jbns = 1;
        }
        float SUM = B_AC[ibsl];
        if (jbns <= ibsl - 1) {
            for (int kbns = jbns; kbns <= ibsl - 1; ++kbns) {
                SUM = SUM - U_SFF[kbns][ibsl - kbns + 1] * X_DF[kbns];
            }
        }
        X_DF[ibsl] = SUM;
    }

    for (int ibsl = 1; ibsl <= N; ++ibsl) {
        X_DF[ibsl] = X_DF[ibsl] / U_SFF[ibsl][1];
    }

    for (int I1 = 1; I1 <= N; ++I1) {
        int ibsl = N - I1 + 1;
        int jbns = ibsl + NB - 1;
        if (jbns > N) {
            jbns = N;
        }
        float SUM = X_DF[ibsl];
        if (ibsl + 1 <= jbns) {
            for (int kbns = ibsl + 1; kbns <= jbns; ++kbns) {
                SUM = SUM - U_SFF[ibsl][kbns - ibsl + 1] * X_DF[kbns];
            }
        }
        X_DF[ibsl] = SUM;
    }
}

} // namespace orcisf::engine
