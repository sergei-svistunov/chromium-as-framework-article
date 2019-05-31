#include <iostream>

#include "base/at_exit.h"
#include "base/task/thread_pool/thread_pool.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "net/url_request/url_request_context_builder.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/redirect_info.h"

#define BUF_SZ 100 * 1024

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

int main(int argc, char **argv) {
    base::CommandLine::Init(argc, argv);

    auto args = base::CommandLine::ForCurrentProcess()->GetArgs();

    if (args.empty()) {
        std::cerr << "Usage: " << base::CommandLine::ForCurrentProcess()->GetProgram() << " <URL>" << std::endl;
        return 1;
    }

    base::AtExitManager exit_manager;

    base::ThreadPool::CreateAndStartWithDefaultParams("downloader");
    
	base::MessageLoop msg_loop(base::MessageLoop::TYPE_IO);
    base::RunLoop run_loop;

    auto ctx = net::URLRequestContextBuilder().Build();

    MyDelegate delegate(run_loop.QuitClosure());

    auto req = ctx->CreateRequest(GURL(args[0]), net::RequestPriority::DEFAULT_PRIORITY, &delegate);
    req->Start();

    run_loop.Run();

    return 0;
}