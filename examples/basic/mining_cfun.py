import desbordante
import pandas as pd
import time

MINIMUM_SUPPORT = 2     # Минимальное количество кортежей, которые должен удовлетворять шаблон
TABLE_PATH = 'examples/datasets/cfd_datasets/city.csv' # Путь к тестовому датасету

class Colors:
    """Класс для цветного вывода в консоль"""
    GREEN_BG = '\033[1;42m'
    GREEN_FG = '\033[1;32m'
    RED_BG = '\033[1;41m'
    RED_FG = '\033[1;31m'
    BLUE_FG = '\033[1;34m'
    YELLOW_BG = '\033[1;43m'
    DEFAULT_BG = '\033[1;49m'
    DEFAULT_FG = '\033[1;37m'
    RESET = '\033[0m'


def note():
    print(f"{Colors.YELLOW_BG}===Note==={Colors.RESET}")
    print("Важно отметить, что существует несколько определений CFD в академической литературе.")
    print("Мы рекомендуем ознакомиться с другими нашими примерами, которые используют другое определение CFD,")
    print("чтобы получить более полное представление о концепции:")
    print(" 1.mining_cfd.py")
    print(" 2.verifying_cfd.py")
    print()
    print()

def explain_cfd_concept():
    """Объяснение концепции условных функциональных зависимостей"""
    print(f"{Colors.BLUE_FG}=== Понимание условных функциональных зависимостей (CFD) ==={Colors.RESET}\n")
    
    print("Условная функциональная зависимость (CFD) расширяет классическую функциональную")
    print("зависимость, определяя область её действия с помощью условий на атрибуты.")
    print()
    print("В данном примере мы используем определение константных CFD из статьи:")
    print(f"{Colors.GREEN_FG}'Discovering (frequent) constant conditional functional dependencies'")
    print(f"Thierno Diallo, Jérôme David, Carine Souveyet, и autres.{Colors.RESET}")
    print()
    print("Согласно этому определению, CFD — это правило вида (X -> Y, T_p), где:")
    print("  X -> Y --- обычная функциональная зависимость")
    print("  T_p --- таблица с атрибутами X и Y, определяющая область действия зависимости.")
    print("  Таблица T_p состоит из шаблонов, которые содержат конкретные значения из домена соответствующих атрибутов")
    print("Пример:")
    print(" [Department, Position] -> Location")
    print("     .IT..Engineer. (Moscow)")
    print("     .HR..Specialist. (Saint_Petersburg)")
    print("Это правило означает, что все IT-инженеры работают в Москве, а все HR-специалисты - в Санкт-Петербурге.")
    print()


def explain_cfun_algorithm():
    """Описание алгоритма CFUN"""
    print(f"{Colors.BLUE_FG}=== Алгоритм CFUN ==={Colors.RESET}\n")
    
    print("CFUN - алгоритм для поиска константных CFD.")
    print("Его основные особенности:")
    print(" 1. Поиск минимального множества CFD, которые покрывают все наблюдаемые зависимости в данных.")
    print(" 2. Можно задать минимальное количество строк, которые должен поддерживать каждый шаблон в T_p.")
    print()
    print()


def load_and_explain_dataset():
    """Загрузка и объяснение структуры датасета"""
    print(f"{Colors.BLUE_FG}=== Загрузка и анализ датасета ==={Colors.RESET}\n")
    
    df = pd.read_csv(TABLE_PATH)
    print(f"Датасет загружен из: {TABLE_PATH}")


    print("\nСодержимое датасета:")
    print(df.to_string())
    print()
    print("Описание датасета:")
    print("Этот датасет содержит информацию об объектах недвижимости в трёх крупных городах США,")
    print("включая название улицы, почтовый индекс, тип строения и уровень стоимости строительства.")
    print()

    print("Ожидаемые УФЗ в этом датасете:")
    
    print("1. (Department='IT', Position='Engineer') -> (Location='Moscow')")
    print("   Все IT-инженеры работают в Москве")
    print()
    print("2. (Department='HR', Position='Specialist') -> (Location='LA')")
    print("   Все HR-специалисты работают в LA")
    print()
    
    return df


def discover_cfds_with_cfun(df, min_support):
    """Поиск УФЗ с помощью алгоритма CFUN"""
    print(f"{Colors.BLUE_FG}=== Поиск условных функциональных зависимостей ==={Colors.RESET}\n")
    
    print(f"Параметры поиска:")
    print(f"* Минимальная поддержка (min_sup): {min_support}")
    print()
    
    
    # Создаем и настраиваем алгоритм CFUN
    # В Desbordante используется единый интерфейс для CFD алгоритмов
    algo = desbordante.cfd.algorithms.CFUN()
    algo.load_data(table=df)
    algo.execute(cfd_minsup=min_support)
    
    cfds = algo.get_cfds()
    
    print(f"Найдено УФЗ: {len(cfds)}")
    print()
    
    if cfds:
        print("Список найденных УФЗ:")
        for i, cfd in enumerate(cfds, 1):
            print(f"  {i}. {cfd}")
        print()
    else:
        print("УФЗ с заданными параметрами не найдены.")
        print("Попробуйте уменьшить min_support.")
        print()
    
    return cfds


def analyze_cfd_metrics(df, cfds):
    """Анализ метрик конкретной УФЗ"""
    print(f"{Colors.GREEN_FG}Анализ УФЗ: {cfd}{Colors.RESET}")
    
    verifier = desbordante.cfd_verification.algorithms.Default()
    verifier.load_data(table=df)
    verifier.execute(cfd_rule=cfd, minconf=0)
    
    holds = verifier.cfd_holds()
    support = verifier.get_real_support()
    confidence = verifier.get_real_confidence()
    violations = verifier.get_num_rows_violating_cfd()
    
    print(f"  УФЗ выполняется: {Colors.GREEN_FG if holds else Colors.RED_FG}{holds}{Colors.RESET}")
    print(f"  Поддержка: {support}")
    print(f"  Уверенность: {confidence:.3f}")
    print(f"  Нарушений: {Colors.RED_FG if violations > 0 else Colors.GREEN_FG}{violations} строк{Colors.RESET}")
    print()
    
    return holds, support, confidence, violations


def compare_different_parameters(df):
    """Сравнение результатов при разных параметрах"""
    print(f"{Colors.BLUE_FG}=== Влияние параметров на результат ==={Colors.RESET}\n")
    
    param_sets = [
        {"support": 2, "confidence": 0.6, "max_lhs": 1, "name": "Низкие требования"},
        {"support": 3, "confidence": 0.8, "max_lhs": 2, "name": "Средние требования"},
        {"support": 5, "confidence": 0.95, "max_lhs": 2, "name": "Высокие требования"}
    ]
    
    results = []
    for params in param_sets:
        print(f"{Colors.YELLOW_BG}Параметры: {params['name']}{Colors.RESET}")
        print(f"  minsup={params['support']}, minconf={params['confidence']}, max_lhs={params['max_lhs']}")
        
        algo = desbordante.cfd.algorithms.Default()
        algo.load_data(table=df)
        algo.execute(
            cfd_minsup=params["support"],
            cfd_minconf=params["confidence"],
            cfd_max_lhs=params["max_lhs"]
        )
        
        cfds = algo.get_cfds()
        print(f"  Найдено УФЗ: {len(cfds)}")
        results.append((params["name"], len(cfds)))
        print()
    
    print("Выводы о влиянии параметров:")
    print("* Увеличение минимальной поддержки и уверенности уменьшает количество УФЗ")
    print("* Увеличение max_lhs позволяет находить более сложные зависимости")
    print("* Для поиска специфичных правил лучше использовать низкую поддержку")
    print("* Для поиска надежных правил — высокую уверенность")
    print()
    
    return results

def main():
    """Основная функция демонстрации"""
    print(f"{Colors.BLUE_FG}=== Демонстрация алгоритма CFUN для поиска УФЗ ==={Colors.RESET}\n")

    note()

    explain_cfd_concept()

    explain_cfun_algorithm()

    df = load_and_explain_dataset()

        
    # Поиск УФЗ с параметрами по умолчанию
    cfds = discover_cfds_with_cfun(df, MINIMUM_SUPPORT)
    
    # Анализ найденных УФЗ
    # if cfds:
    #     analyze_cfd_metrics(df, cfds)
    
    # # Сравнение параметров
    # compare_different_parameters(df)
    
    # Практические рекомендации
    print(f"{Colors.BLUE_FG}=== Практические рекомендации ==={Colors.RESET}\n")
    print("1. Для поиска общих закономерностей используйте высокую поддержку")
    print("2. Для поиска редких, но важных правил — низкую поддержку")
    print("3. Уверенность 0.8-0.95 обычно дает хорошие результаты")
    print("4. Начинайте с max_lhs=1-2, затем увеличивайте при необходимости")
    print()
    
    print(f"{Colors.GREEN_FG}=== Заключение ==={Colors.RESET}\n")
    print("В этом примере мы продемонстрировали:")
    print("* Концепцию условных функциональных зависимостей")
    print("* Использование алгоритма CFUN из библиотеки Desbordante")
    print("* Влияние параметров на результат поиска")
    print("* Анализ найденных УФЗ и их метрик")
    print()
    
    print("Другие примеры использования Desbordante можно найти в официальной документации:")
    print("* GitHub: https://github.com/Desbordante/desbordante-core")
    print("* PyPI: https://pypi.org/project/desbordante/")
    print()
    
    print(f"{Colors.GREEN_FG}Пример успешно выполнен!{Colors.RESET}")


if __name__ == '__main__':
    main()