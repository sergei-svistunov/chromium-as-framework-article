# Chromium - это не только браузер, но и хороший фреймворк

Большинство людей привыкли, что Chromium - это браузер и основа для других браузеров. До недавнего времени я был в их числе,
но присоединвшись пару месяцев назад в качестве технического менеджера к команде, которая разрабатывает браузер, я начал открывать другой дивный мир.
Chromium - это огромная экосистема, в которой есть всё: и система зависимостей, и система кросплатформенной сборки, и компоненты почти на все случаи жизни. 
Так почему же не попробовать создавать свои приложения, используя всю эту мощь?

Под катом небольшой туториал, как начать это делать.

## Подготовка окружения

В статье я буду использовать Ubuntu 18.04, для других ОС шаги можно посмотреть в документации:
* [Linux](https://chromium.googlesource.com/chromium/src/+/master/docs/linux_build_instructions.md)
* [Windows](https://chromium.googlesource.com/chromium/src/+/master/docs/windows_build_instructions.md)
* [Mac](https://chromium.googlesource.com/chromium/src/+/master/docs/mac_build_instructions.md)

Для выполнения последующих шагов необходимы Git и Python. Если они не установлены, то их необходимо поставить с помощью команды:
```bash
sudo apt install git python
```

### Установка `depot_tools`
`depot_tools` - это набор инструментов для разработки Chromium: <!--- Tools for working with Chromium development -->. Для его установки необходимо выполнить
```bash
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
```
и добавить путь в переменную окружения `PATH`:
```bash
export PATH="$PATH:/path/to/depot_tools"
```
**Важно:** если `depot_tools` были скачаны в домашнюю папку, то не используйте `~` в переменной `PATH`, иначе могут возникнуть проблемы. Необходимо использовать переменную `$HOME`:
```bash
export PATH="$PATH:${HOME}/depot_tools"
```

### Получение кода

Для начала надо создать папку, где будут лежать исходники и перейти в неё. Например в домашней директории (необходимо около 30Gb свободного места):
```bash
mkdir ~/chromium && cd ~/chromium
```

После этого можно скачать исходники с помощью утилиты `fetch` из `depot_tools`:
```bash
fetch --nohooks --no-history chromium
```
Теперь можно пойти попить чай/кофе, так как процедура небыстрая. Для экспериментов история не нужна, поэтому используется флаг `--no-history`, с историей будет ещё дольше.

### Установка зависимостей
Все исходники лежат в папке `src`, идём в неё:
```bash
cd src
```

Теперь нужно поставить все зависимости с помощью скрипта:
```bash
./build/install-build-deps.sh
```
и запустить хуки:
```bash
gclient runhooks
```
На этом подготовка окружения завершена.

## Система сборки
Для сборки Chromium используются [Ninja](https://ninja-build.org/), в качестве основной системы сборки, и [GN](https://gn.googlesource.com/gn/+/master/docs/quick_start.md) - утилита для генерации `.ninja` файлов.

Чтобы понять как ими пользоваться, предлагаю создать тестовую утилиту `example`. Для этого в папке `src` надо создать подпапку `example`:
```bash
mkdir example
```

Затем в папке `src/example` надо создать файл `BUILD.gn`, который содержит
```
executable("example") {
 sources = [
   "example.cc",
 ]
}
```
`BUILD.gn` состоит из цели - исполняемый файл `example` и списка файлов, которые необходимы для сборки цели.

Следющим шагом надо создать сам файл `example.cc`. Для начала предлагаю создать классическое приложение "Hello world":
```c++
#include <iostream>

int main(int argc, char **argv) {
   std::cout << "Hello world" << std::endl;

   return 0;
}
```
Исходный код можно найти на [GitHub](https://github.com/sergei-svistunov/chromium-as-framework-article/tree/example-1).

Чтобы `GN` узнала о новом проекте, нужно в файле `BUILD.gn`, который находится в `src`, в разделе `deps` добавить добавить строку `"//example"`:
```
...
group("gn_all") {
 testonly = true

 deps = [
   ":gn_visibility",
   "//base:base_perftests",
   "//base:base_unittests",
   "//base/util:base_util_unittests",
   "//chrome/installer",
   "//chrome/updater",
   "//net:net_unittests",
   "//services:services_unittests",
   "//services/service_manager/public/cpp",
   "//skia:skia_unittests",
   "//sql:sql_unittests",
   "//third_party/flatbuffers:flatbuffers_unittests",
   "//tools/binary_size:binary_size_trybot_py",
   "//tools/ipc_fuzzer:ipc_fuzzer_all",
   "//tools/metrics:metrics_metadata",
   "//ui/base:ui_base_unittests",
   "//ui/gfx:gfx_unittests",
   "//url:url_unittests",

   # ↓↓↓↓↓↓↓↓
   "//example",
 ]
 ...
```

Теперь можно переходить к генерации проекта. Для этого необходимо вернуться в папку `src` и сгенерировать проект с помощью команды
```bash
gn gen out/Default
```

`GN` также позволяет подготовить проект для одной из поддерживаемых IDE:
* eclipse
* vs
* vs2013
* vs2015
* vs2017
* vs2019
* xcode
* qtcreator
* json

Более подробную информацию можно получить с помощью команды
```bash
gn help gen
```

Например для работы с проектом `example` в [QtCreator](https://www.qt.io/download) надо выполнить команду
```bash
gn gen --ide=qtcreator --root-target=example out/Default
```
После этого можно его открыть в `QtCreator`:
```bash
qtcreator out/Default/qtcreator_project/all.creator
```

Следующий и финальный шаг - сборка проекта с помощью `Ninja`:
```bash
autoninja -C out/Default example
```

На этом краткое ознакомление с системой сборки можно завершить. 

Приложение можно запустить с помощью команды
```bash
./out/Default/example
```
и увидеть `Hello world`.

На самом деле про систему сборки используемую в Chromium можно написать отдельную статью, возможно и не одну.

## Работа с командной строкой

В качестве первого примера использования кодовой базы Chromium как фреймворка предлагаю "поиграться" с командной строкой.

**Задача:** *Вывести на экран все аргументы переданные приложению в стиле Chromium.*

Для работы с командной строкой необходимо подключить заголовочный файл в `example.cc`:
```c++
`#include "base/command_line.h"
```
А также надо не забыть в `BUILD.gn` добавить зависимость от проекта `base`. `BUILD.gn` должен выглядеть так:
```
executable("example") {
 sources = [
   "example.cc",
 ]

 deps = [
   "//base",
 ]
}
```
Теперь всё необходимое будет прилинковано к `example`.

Для работы с командной строкой Chromium предоставляет синглтон `base::CommandLine`. Чтобы получить ссылку на него, надо использовать статический метод `base::CommandLine::ForCurrentProcess`, но сначала надо его инициализировать с помощью метода `base::CommandLine::Init`:
```c++
base::CommandLine::Init(argc, argv);

auto *cmd_line = base::CommandLine::ForCurrentProcess();
```

Все аргументы переданные приложению в командной строке начинающиеся с символа `-` возвращаются в виде `base::SwitchMap` (по сути `map<string, string>`) с помощью метода `GetSwitches`. Все остальные аргументы возвращаются в виде `base::StringVector` (по сути `vector<string>)`. Этих знаний достаточно, чтобы реализовать код для задачи:
```c++
for (const auto &sw : cmd_line->GetSwitches()) {
   std::cout << "Switch " << sw.first << ": " << sw.second << std::endl;
}

for (const auto &arg: cmd_line->GetArgs()) {
   std::cout << "Arg " << arg << std::endl;
}
```
Полную версию можно найти на [GitHub](https://github.com/sergei-svistunov/chromium-as-framework-article/tree/example-2).

Чтобы собрать и запустить приложение надо выполнить:
```c++
autoninja -C out/Default example
./out/Default/example arg1 --sw1=val1 --sw2 arg2
```
На экран будет выведено:
```
Switch sw1: val1
Switch sw2:
Arg arg1
Arg arg2
```

## Работа с сетью

В качестве второго и последнего на сегодня примера предлагаю поработать с сетевой частью Chromium.

**Задача:** *Вывести на экран содержимое URL'а, переданного в качестве аргумента.*

### Сетевая подсистема Chromium

Сетевая подсистема довольно большая и сложная. Входной точкой для запросов к HTTP, HTTPS, FTP и другим data-ресурсам является `URLRequest`,
который уже определяет какой из клиентов задействовать.
Упрощённая схема выглядит так:

![](Chromium%20HTTP%20Network%20Request%20Diagram.svg)

*Полную версию можно посмотреть в [документации](https://chromium.googlesource.com/chromium/src/+/master/net/docs/life-of-a-url-request.md).*

Для создания `URLRequest'а` необходимо использовать `URLRequestContext`. Создание контекста довольно сложная операция, поэтому рекомендуется использовать `URLRequestContextBuilder`.
Он проинициализирует все необходимые переменные значениями по умолчанию, но при желании их можно поменять на свои, например:
```c++
net::URLRequestContextBuilder context_builder;

context_builder.DisableHttpCache();
context_builder.SetSpdyAndQuicEnabled(true /* http2 */, false /* quic */);
context_builder.SetCookieStore(nullptr);
```

### Многопоточность

Сетевой стэк Chromium расчитан на работу в многопоточной среде, поэтому пропустить эту тему нельзя.

Базовыми объектами для работы с многопоточностью в Chromium являются:
* **Task:** Задача для выполнения, в Chromium это фунция с типом `base::Callback`, которую можно создать с помощью `base::Bind`.
* **Task queue:** Очередь задач для выполнения.
* **Physical thread:** Кроссплатформенная обёртка над потоком операционной системы (pthread в POSIX или CreateThread() в Windows).
Реализовано в классе `base::PlatformThread`, не используйте напрямую.
* **base::Thread:** Реальный поток, который бесконечно обрабатывает сообщения от выделенной очереди задач, не рекомендуется создавать их напрямую.
* **Thread pool:** Пул потоков с общей очередью задач. Реализован в классе `base::ThreadPool`. Как правило создают один экземпляр. Задачи в него отправляются с помощью функций из `base/task/post_task.h`.
* **Sequence or Virtual thread:** Виртуальный поток, который использует реальные потоки и может переключаться между ними.
* **Task runner:** Интерфейс для постановки задач, реализован в классе `base::TaskRunner`.
* **Sequenced task runner:** Интерфейс для постановки задач, который гарантирует, что задачи будут исполнены в том же порядке, в каком попали. Реализовано в классе `base::SequencedTaskRunner`.
* **Single-thread task runner:** Аналогичен предыдущему, но гарантирует, что все задачи будут выполнены в одном потоке ОС. Реализовано в классе `base::SingleThreadTaskRunner`.

### Реализация

Некоторые компоненты Chromium требуют наличия `base::AtExitManager` - класс, который позволяет зарегистрировать операции, которые надо выполнить при завершении приложения.
Использовать его очень просто, необходимо создать объект на стеке:
```c++
base::AtExitManager exit_manager;
```
Когда `exit_manager` выйдет из области видимости, все зарегистрированные callback'и будут выполнены.

Теперь нужно позаботиться о наличии всех необходимых компонент многопоточности для сетевой подсистемы.
Для этого нужно создать `Thread pool`, а также `Message loop` с типом `TYPE_IO` для обработки сетевых сообщений и
`Run loop` - основной цикл программы:
```c++
base::ThreadPool::CreateAndStartWithDefaultParams("downloader");
  
base::MessageLoop msg_loop(base::MessageLoop::TYPE_IO);
base::RunLoop run_loop;
```

Дальше нужно создать `Context`, с помощью `Context builder'а`:
```c++
auto ctx = net::URLRequestContextBuilder().Build();
```

Чтобы послать запрос, необходимо создать объект `URLRequest` с помощью метода `CreateRequest` объекта `ctx`.
В качестве параметров передаются:
* URL, строка с типом `GURL`.
* Приоритет.
* Делегат, который обрабатывает события.

Делегат представляет собой класс, реализующий интерфейс `net::URLRequest::Delegate`. Для данной задачи он может выглядеть так:
```c++
class MyDelegate : public net::URLRequest::Delegate {
public:
   explicit MyDelegate(base::Closure quit_closure) : quit_closure_(std::move(quit_closure)),
                                                     buf_(base::MakeRefCounted<net::IOBuffer>(BUF_SZ)) {}

   void OnReceivedRedirect(net::URLRequest *request, const net::RedirectInfo &redirect_info,
                           bool *defer_redirect) override {
       std::cerr << "redirect to " << redirect_info.new_url << std::endl;
   }

  void OnAuthRequired(net::URLRequest* request, const net::AuthChallengeInfo& auth_info) override {
       std::cerr << "auth req" << std::endl;
   }

   void OnCertificateRequested(net::URLRequest *request, net::SSLCertRequestInfo *cert_request_info) override {
       std::cerr << "cert req" << std::endl;
   }

   void OnSSLCertificateError(net::URLRequest* request, int net_error, const net::SSLInfo& ssl_info, bool fatal) override {
       std::cerr << "cert err" << std::endl;
   }

   void OnResponseStarted(net::URLRequest *request, int net_error) override {
       std::cerr << "resp started" << std::endl;
       while (true) {
           auto n = request->Read(buf_.get(), BUF_SZ);
           std::cerr << "resp read " << n << std::endl;

           if (n == net::ERR_IO_PENDING)
               return;

           if (n <= 0) {
               OnReadCompleted(request, n);
               return;
           }

           std::cout << std::string(buf_->data(), n) << std::endl;
       }
   }

   void OnReadCompleted(net::URLRequest *request, int bytes_read) override {
       std::cerr << "completed" << std::endl;
       quit_closure_.Run();
   }

private:
   base::Closure quit_closure_;
   scoped_refptr<net::IOBuffer> buf_;
};
```

Вся основная логика происходит в обработчике события `OnResponseStarted`: вычитывается содержимое ответа пока не произойдёт ошибка
или будет нечего читать.
Так как после чтения ответа нужно завершить приложение, то делегат должен иметь доступ к функции, которая прервёт основной `Run loop`,
в данном случае использует callback типа `base::Closure`.

Теперь всё готово для отправки запроса:
```c++
MyDelegate delegate(run_loop.QuitClosure());

auto req = ctx->CreateRequest(GURL(args[0]), net::RequestPriority::DEFAULT_PRIORITY, &delegate);
req->Start();
```

Чтобы запрос начал обрабатываться, надо зупустить `Run loop`:
```c++
run_loop.Run();
```

Полную версию можно найти на [GitHub](https://github.com/sergei-svistunov/chromium-as-framework-article/tree/example-3).

Чтобы собрать и запустить приложение нужно выполнить:
```bash
autoninja -C out/Default example
out/Default/example "https://example.com/"
```

## Финал

На самом деле в Chromium можно найти много полезных "кубиков" и "кирпичиков" из которых можно строить приложения.
Он постоянно развивается, что с одной стороны является плюсом, а с другой стороны, регулярные изменения API не дают расслабиться.
Например, в последнем релизе `base::TaskScheduler` превратился в `base::ThreadPool`, к счастью без изменения API.