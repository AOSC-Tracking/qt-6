import { HTTPRequest } from '../api/HTTPRequest.js';
import { UnsupportedOperation } from '../common/Errors.js';
/**
 * @internal
 */
export class BidiHTTPRequest extends HTTPRequest {
    _response = null;
    _redirectChain;
    _navigationId;
    #url;
    #resourceType;
    #method;
    #postData;
    #headers = {};
    #initiator;
    #frame;
    constructor(event, frame, redirectChain = []) {
        super();
        this.#url = event.request.url;
        this.#resourceType = event.initiator.type.toLowerCase();
        this.#method = event.request.method;
        this.#postData = undefined;
        this.#initiator = event.initiator;
        this.#frame = frame;
        this._requestId = event.request.request;
        this._redirectChain = redirectChain;
        this._navigationId = event.navigation;
        for (const header of event.request.headers) {
            // TODO: How to handle Binary Headers
            // https://w3c.github.io/webdriver-bidi/#type-network-Header
            if (header.value.type === 'string') {
                this.#headers[header.name.toLowerCase()] = header.value.value;
            }
        }
    }
    get client() {
        throw new UnsupportedOperation();
    }
    url() {
        return this.#url;
    }
    resourceType() {
        return this.#resourceType;
    }
    method() {
        return this.#method;
    }
    postData() {
        return this.#postData;
    }
    hasPostData() {
        return this.#postData !== undefined;
    }
    async fetchPostData() {
        return this.#postData;
    }
    headers() {
        return this.#headers;
    }
    response() {
        return this._response;
    }
    isNavigationRequest() {
        return Boolean(this._navigationId);
    }
    initiator() {
        return this.#initiator;
    }
    redirectChain() {
        return this._redirectChain.slice();
    }
    enqueueInterceptAction(pendingHandler) {
        // Execute the handler when interception is not supported
        void pendingHandler();
    }
    frame() {
        return this.#frame;
    }
    continueRequestOverrides() {
        throw new UnsupportedOperation();
    }
    continue(_overrides = {}) {
        throw new UnsupportedOperation();
    }
    responseForRequest() {
        throw new UnsupportedOperation();
    }
    abortErrorReason() {
        throw new UnsupportedOperation();
    }
    interceptResolutionState() {
        throw new UnsupportedOperation();
    }
    isInterceptResolutionHandled() {
        throw new UnsupportedOperation();
    }
    finalizeInterceptions() {
        throw new UnsupportedOperation();
    }
    abort() {
        throw new UnsupportedOperation();
    }
    respond(_response, _priority) {
        throw new UnsupportedOperation();
    }
    failure() {
        throw new UnsupportedOperation();
    }
}
//# sourceMappingURL=HTTPRequest.js.map