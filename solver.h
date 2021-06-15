#include <vector>
#include <iostream>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>

#define eps 1e-7

using namespace std;

class solver {
private:
    SCIP *scip;
    size_t prev_added;
    vector<SCIP_VAR *> vars;
    vector<SCIP_CONS *> cons;
    char buff[sizeof(size_t)];

public:
    explicit solver(size_t edges) : scip(nullptr), prev_added(0), vars(edges + 1), buff() {
        SCIP_CALL_ABORT(SCIPcreate(&scip));
        SCIP_CALL_ABORT(SCIPincludeDefaultPlugins(scip));
        SCIP_CALL_ABORT(SCIPcreateProbBasic(scip, "NE"));
//        SCIP_CALL_ABORT(SCIPsetIntParam(scip, "display/verblevel", 0));
//        SCIP_CALL_ABORT(SCIPsetEmphasis(scip, SCIP_PARAMEMPHASIS_CPSOLVER, true));
        SCIP_CALL_ABORT(SCIPsetRealParam(scip, "numerics/feastol", eps / 2));
        for (size_t i = 0; i != vars.size(); ++i) {
            SCIP_VAR *var = nullptr;
            SCIP_CALL_ABORT(SCIPcreateVarBasic(scip, &var, nullptr, 0, 1, 1, SCIP_VARTYPE_CONTINUOUS));
            SCIP_CALL_ABORT(SCIPaddVar(scip, var));
            vars[i] = var;
        }
        SCIP_Bool fixed, infeasible;
        SCIP_CALL_ABORT(SCIPfixVar(scip, vars.front(), eps, &infeasible, &fixed));
        assert(fixed == true);
        assert(infeasible == false);
    }

    void make_cons_name() {
        *reinterpret_cast<size_t *>(buff) = cons.size();
    }

    void add_linear_cons() {
        make_cons_name();
        SCIP_CONS *con = nullptr;
        SCIP_CALL_ABORT(SCIPcreateConsBasicLinear(scip, &con, buff, 0, nullptr, nullptr, -SCIPinfinity(scip), 0));
        SCIP_CALL_ABORT(SCIPaddCoefLinear(scip, con, vars.front(), 1));
        cons.push_back(con);
    }

    void add_linear_coef(size_t var, SCIP_Real coef) {
        SCIP_CALL_ABORT(SCIPaddCoefLinear(scip, cons.back(), vars[var + 1], coef));
    }

    void add_or_clause() {
        for (size_t i = prev_added; i != cons.size(); ++i) {
            SCIP_CALL_ABORT(SCIPaddCons(scip, cons[i]));
        }
        make_cons_name();
        size_t con_n = cons.size() - prev_added;
        SCIP_CONS *con = nullptr, **beg = cons.data() + prev_added;
        SCIP_CALL_ABORT(SCIPcreateConsBasicDisjunction(scip, &con, buff, con_n, beg, nullptr));
        SCIP_CALL_ABORT(SCIPaddCons(scip, con));
        cons.push_back(con);
        prev_added += con_n;
    }

    bool solve() {
        SCIP_CALL_ABORT(SCIPsolve(scip));
        SCIP_STATUS res = SCIPgetStatus(scip);
        if (res == SCIP_STATUS_INFEASIBLE) {
            return false;
        } else if (res == SCIP_STATUS_OPTIMAL) {
            SCIP_SOL *sol = SCIPgetBestSol(scip);
            for (auto var: vars) {
                cout << SCIPgetSolVal(scip, sol, var) << ' ';
            }
            cout << '\n';
            return true;
        } else {
            exit(1);
        }
    }

    ~solver() {
        for (auto &var : vars) {
            SCIP_CALL_ABORT(SCIPreleaseVar(scip, &var));
        }
        for (auto &con : cons) {
            SCIP_CALL_ABORT(SCIPreleaseCons(scip, &con));
        }
        SCIP_CALL_ABORT(SCIPfree(&scip));
    }
};
