# Agents general instruction

- This is the project folder for a dungeon game.
- Read [game desciption](/documents/game-description.md) and [architecture](/documents/architecture.puml) for contexts about the game; [README](README.md) for file structure and general coding conventions.
- Read [ui-framework-usage](/documents/ui-framework-usage.md) for tutorial on UI framework usage.
- Remember to split prototypes and implemetation into separate `.cpp` and `.hpp` files.
- Implementations must meet strict OOP rules.

---

## Core Principles (SOLID)

1. **Single Responsibility** — Every class must have exactly one reason to change. If a class handles more than one concern (e.g., data validation + persistence), split it.
2. **Open/Closed** — Classes should be extendable via inheritance/composition without modifying existing, tested code. Never edit a working class to bolt on unrelated behavior — extend it.
3. **Liskov Substitution** — Any subclass must be usable anywhere its parent is expected, without breaking behavior. Never override a method in a way that narrows preconditions or weakens postconditions.
4. **Interface Segregation** — Never force a class to implement methods it doesn't use. Split fat interfaces into smaller, role-specific ones.
5. **Dependency Inversion** — High-level modules must depend on abstractions (interfaces/protocols), not concrete implementations. Inject dependencies rather than instantiating them inside a class.

## Encapsulation

6. All fields default to **private**; expose only through methods/properties that enforce invariants.
7. Never expose mutable internal state directly (no returning raw references to internal lists/dicts — return copies or read-only views).
8. Getters/setters must not be blindly generated for every field — only expose what's genuinely needed externally.
9. Validate all inputs at the boundary of a class (constructors and public methods), not deep inside private logic.

## Inheritance

10. Use inheritance only for true "is-a" relationships — never for code reuse alone ("has-a" → composition).
11. Favor **composition over inheritance** by default; inheritance is the exception, not the rule.
12. No inheritance depth beyond ~2–3 levels without strong justification.
13. Never override a method just to disable/no-op it — that signals a Liskov violation and the hierarchy is wrong.
14. Mark base class methods as `abstract`/`virtual` explicitly when subclasses are expected to override them; otherwise mark `final`/sealed.

## Polymorphism

15. Prefer polymorphic dispatch over `if/elif` or `switch` chains that branch on type. If you see type-checking logic (`isinstance`, `instanceof`) driving behavior, refactor to a virtual method.
16. Every overridden method must honor the parent's contract (same semantic meaning, compatible return/error behavior).

## Abstraction

17. Depend on interfaces/abstract base classes, not concrete classes, when defining collaborators.
18. Keep abstractions free of implementation details — an interface should never leak the internals of one specific implementation.
19. Don't create an abstraction for a single implementation "just in case" — YAGNI applies; introduce the interface when the second implementation actually appears.

## Class Design

20. One public class per file/module unless tightly coupled helper classes are private to it.
21. Constructors do setup only — no I/O, no side effects beyond initializing state.
22. Immutable by default: prefer read-only/final fields unless mutation is required.
23. No god classes — enforce a soft limit (e.g., ~200–300 lines or ~7±2 public methods) as a refactor trigger.
24. No public fields; no static mutable shared state (avoid hidden global coupling).
25. Fail fast: validate invariants in constructors/setters and raise/throw immediately rather than allowing an invalid object to exist.

## Naming & Structure

26. Class names are nouns; method names are verbs; boolean methods/properties read as predicates (`isValid`, `hasPermission`).
27. Consistent access modifiers — never mix conventions (e.g., leading underscore vs. explicit `private`) within the same codebase.
28. Every class should be describable in one sentence without "and" — if you need "and," split it (ties back to SRP).

## Testing/Verification Discipline

29. Every public method must be unit-testable in isolation via dependency injection (no hardwired singletons/statics that block mocking).
30. No circular dependencies between classes or modules — if class A needs B and B needs A, extract a shared abstraction.