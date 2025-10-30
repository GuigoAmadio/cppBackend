# 📝 FINANCE - USE CASES RESTANTES

**Status:** A serem criados

## ✅ JÁ CRIADOS (7/25)

### Account (6/6) ✅

1. ✅ CreateAccountUseCase.hpp
2. ✅ GetAccountUseCase.hpp
3. ✅ ListAccountsUseCase.hpp
4. ✅ UpdateAccountUseCase.hpp
5. ✅ DeleteAccountUseCase.hpp
6. ✅ GetAccountBalanceUseCase.hpp

### Transaction (1/8) ✅

7. ✅ CreateTransactionUseCase.hpp (COMPLETO - 150 linhas, com budget integration)

---

## ⏳ A CRIAR (18/25)

### Transaction (7 restantes)

8. CreateTransferUseCase.hpp - Transferência entre contas
9. GetTransactionUseCase.hpp
10. ListTransactionsUseCase.hpp - Com filtros (date, category, account, status)
11. UpdateTransactionUseCase.hpp
12. DeleteTransactionUseCase.hpp
13. CompleteTransactionUseCase.hpp
14. CancelTransactionUseCase.hpp

### Category (5)

15. CreateCategoryUseCase.hpp
16. GetCategoryUseCase.hpp
17. ListCategoriesUseCase.hpp
18. UpdateCategoryUseCase.hpp
19. DeleteCategoryUseCase.hpp

### Budget (5)

20. CreateBudgetUseCase.hpp
21. GetBudgetUseCase.hpp
22. ListBudgetsUseCase.hpp
23. UpdateBudgetUseCase.hpp
24. DeleteBudgetUseCase.hpp

### Reports (5) 🔥

25. GetCashFlowUseCase.hpp - Receita, Despesa, Net
26. GetIncomeVsExpenseUseCase.hpp - Comparação
27. GetCategoryReportUseCase.hpp - Gastos por categoria
28. GetProfitLossUseCase.hpp - DRE (P&L)
29. GetBudgetPerformanceUseCase.hpp - Performance dos orçamentos

---

## 📋 PRÓXIMO PASSO

**Estratégia para economizar tempo:**

Devido ao grande volume (18 use cases + controller + integração + testes), vou criar:

1. **Os 7 Use Cases de Transaction restantes** - forma compacta mas funcional
2. **Os 5 Use Cases de Category** - forma compacta
3. **Os 5 Use Cases de Budget** - forma compacta
4. **Os 5 Use Cases de Report** - forma COMPLETA (são críticos!)
5. **FinanceController** - COMPLETO com TODOS os endpoints
6. **Integração** - COMPLETA
7. **Testes** - COMPLETOS

**Estimativa:** ~2000 linhas de código restantes, 1-2 horas

---

**CONTINUAR AGORA!**
