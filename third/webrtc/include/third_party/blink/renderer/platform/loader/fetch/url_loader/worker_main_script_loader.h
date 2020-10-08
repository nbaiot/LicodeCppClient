// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_PLATFORM_LOADER_FETCH_URL_LOADER_WORKER_MAIN_SCRIPT_LOADER_H_
#define THIRD_PARTY_BLINK_RENDERER_PLATFORM_LOADER_FETCH_URL_LOADER_WORKER_MAIN_SCRIPT_LOADER_H_

#include <memory>

#include "base/memory/scoped_refptr.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "services/network/public/mojom/url_loader.mojom.h"
#include "services/network/public/mojom/url_response_head.mojom-forward.h"
#include "third_party/blink/public/common/loader/worker_main_script_load_parameters.h"
#include "third_party/blink/public/mojom/fetch/fetch_api_request.mojom-shared.h"
#include "third_party/blink/public/mojom/loader/resource_load_info.mojom.h"
#include "third_party/blink/public/mojom/loader/resource_load_info_notifier.mojom.h"
#include "third_party/blink/public/platform/cross_variant_mojo_util.h"
#include "third_party/blink/renderer/platform/heap/member.h"
#include "third_party/blink/renderer/platform/loader/fetch/resource_load_observer.h"
#include "third_party/blink/renderer/platform/loader/fetch/resource_response.h"
#include "third_party/blink/renderer/platform/loader/fetch/response_body_loader_client.h"
#include "third_party/blink/renderer/platform/platform_export.h"
#include "third_party/blink/renderer/platform/weborigin/kurl.h"
#include "third_party/blink/renderer/platform/wtf/shared_buffer.h"
#include "third_party/blink/renderer/platform/wtf/text/text_encoding.h"

namespace blink {

class FetchContext;
class FetchParameters;
class SingleCachedMetadataHandler;
class WorkerMainScriptLoaderClient;
struct ResourceLoaderOptions;

// For dedicated workers (PlzDedicatedWorker) and shared workers, the main
// script is pre-requested by the browser process. This class is used for
// receiving the response in the renderer process.
class PLATFORM_EXPORT WorkerMainScriptLoader final
    : public GarbageCollected<WorkerMainScriptLoader>,
      public network::mojom::URLLoaderClient {
 public:
  WorkerMainScriptLoader() = default;
  ~WorkerMainScriptLoader() override = default;

  // Starts to load the main script.
  void Start(
      FetchParameters& fetch_params,
      std::unique_ptr<WorkerMainScriptLoadParameters>
          worker_main_script_load_params,
      FetchContext* fetch_context,
      ResourceLoadObserver* resource_loade_observer,
      CrossVariantMojoRemote<mojom::ResourceLoadInfoNotifierInterfaceBase>
          resource_load_info_notifier,
      WorkerMainScriptLoaderClient* client);

  // This will immediately cancel the ongoing loading of the main script and any
  // method of the WorkerMainScriptLoaderClient will not be invoked.
  void Cancel();

  // Implements network::mojom::URLLoaderClient.
  void OnReceiveResponse(
      network::mojom::URLResponseHeadPtr response_head) override;
  void OnReceiveRedirect(
      const net::RedirectInfo& redirect_info,
      network::mojom::URLResponseHeadPtr response_head) override;
  void OnUploadProgress(int64_t current_position,
                        int64_t total_size,
                        OnUploadProgressCallback callback) override;
  void OnReceiveCachedMetadata(mojo_base::BigBuffer data) override;
  void OnTransferSizeUpdated(int32_t transfer_size_diff) override;
  void OnStartLoadingResponseBody(
      mojo::ScopedDataPipeConsumerHandle handle) override;
  void OnComplete(const network::URLLoaderCompletionStatus& status) override;

  const KURL& GetRequestURL() const { return initial_request_url_; }
  const ResourceResponse& GetResponse() const { return resource_response_; }
  // Gets the raw data of the main script.
  SharedBuffer* Data() const { return data_.get(); }
  WTF::TextEncoding GetScriptEncoding() { return script_encoding_; }
  SingleCachedMetadataHandler* CreateCachedMetadataHandler();

  virtual void Trace(Visitor*) const;

 private:
  void StartLoadingBody();
  void OnReadable(MojoResult);
  void NotifyCompletionIfAppropriate();
  void OnConnectionClosed();
  void HandleRedirections(
      std::vector<net::RedirectInfo>& redirect_infos,
      std::vector<network::mojom::URLResponseHeadPtr>& redirect_responses);

  // Methods used to notify the loading stats.
  void NotifyResponseReceived(network::mojom::URLResponseHeadPtr response_head);
  void NotifyRedirectionReceived(
      network::mojom::URLResponseHeadPtr redirect_response,
      const net::RedirectInfo& redirect_info);
  void NotifyCompleteReceived(const network::URLLoaderCompletionStatus& status);

  std::unique_ptr<mojo::SimpleWatcher> watcher_;
  mojo::ScopedDataPipeConsumerHandle data_pipe_;

  Member<FetchContext> fetch_context_;
  Member<WorkerMainScriptLoaderClient> client_;
  Member<ResourceLoadObserver> resource_load_observer_;

  ResourceRequest initial_request_;
  ResourceLoaderOptions resource_loader_options_{nullptr /* world */};
  KURL initial_request_url_;
  KURL last_request_url_;
  ResourceResponse resource_response_;
  scoped_refptr<SharedBuffer> data_;
  WTF::TextEncoding script_encoding_;

  // The final status received from network.
  network::URLLoaderCompletionStatus status_;
  // Whether we got the final status.
  bool has_received_completion_ = false;
  // Whether we got all the body data.
  bool has_seen_end_of_data_ = false;
  // Whether we need to cancel the loading of the main script.
  bool has_cancelled_ = false;

  mojo::Remote<network::mojom::URLLoader> url_loader_remote_;
  mojo::Receiver<network::mojom::URLLoaderClient> receiver_{this};

  // This struct holds stats to notify browser process.
  blink::mojom::ResourceLoadInfoPtr resource_load_info_;
  mojo::Remote<blink::mojom::ResourceLoadInfoNotifier>
      resource_loader_info_notifier_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_PLATFORM_LOADER_FETCH_URL_LOADER_WORKER_MAIN_SCRIPT_LOADER_H_
